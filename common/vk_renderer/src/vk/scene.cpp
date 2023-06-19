#include "vk/scene.h"

#include "vk/engine.h"
#include "vk/utils.h"
#include "vk/window.h"
#include "vk/pipeline.h"

#include "renderer/command.h"
#include "utils/log.h"

namespace vk
{
void Scene::UnloadScene()
{
    auto& driver = GetDriver();
    for(const auto& shader : shaders_)
    {
        vkDestroyShaderModule(driver.device, shader.module, nullptr);
    }
    shaders_.clear();
    for(auto& framebuffer: framebuffers_)
    {
        framebuffer.Destroy();
    }
    framebuffers_.clear();
    for (const auto& framebuffer : renderPass_.framebuffers)
    {
        vkDestroyFramebuffer(driver.device, framebuffer, nullptr);
    }
    renderPass_.framebuffers.clear();
    for(auto& drawCommand : drawCommands_)
    {
        drawCommand.Destroy();
    }
    drawCommands_.clear();
    for(auto& command: raytracingCommands_)
    {
        command.Destroy();
    }
    raytracingCommands_.clear();
    for(auto& pipeline : pipelines_)
    {
        pipeline.Destroy();
    }
    pipelines_.clear();
    for(auto& pipeline: raytracingPipelines_)
    {
        pipeline.Destroy();
    }
    raytracingPipelines_.clear();
    vkDestroyRenderPass(driver.device, renderPass_.renderPass, nullptr);
    renderPass_.renderPass = VK_NULL_HANDLE;
    auto& textureManager = core::GetTextureManager();
    textureManager.Clear();
    for(auto& vertexBuffer: vertexBuffers_)
    {
        DestroyBuffer(vertexBuffer.vertexBuffer);
        DestroyBuffer(vertexBuffer.indexBuffer);
    }
    vertexBuffers_.clear();
    for(auto& tlas : topLevelAccelerationStructures_)
    {
        tlas.Destroy();
    }
    topLevelAccelerationStructures_.clear();
    if (raytracingStorageImage_.imageView != VK_NULL_HANDLE)
    {
        vmaDestroyImage(GetAllocator(), raytracingStorageImage_.image.image, raytracingStorageImage_.image.allocation);
        vkDestroyImageView(driver.device, raytracingStorageImage_.imageView, nullptr);
        raytracingStorageImage_ = {};
    }
    
}

void Scene::Update(float dt)
{
    core::Scene::Update(dt);
    auto& renderer = GetRenderer();
    auto& swapchain = GetSwapchain();

    if (renderPass_.renderPass != VK_NULL_HANDLE)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass_.renderPass;
        renderPassInfo.framebuffer = renderPass_.framebuffers[renderer.imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain.extent;

        //Fill with subpass clear values
        std::vector<VkClearValue> clearValues{};
        for (auto& subpass : scene_.render_pass().sub_passes())
        {
            if (subpass.framebuffer_index() == -1 || subpass.framebuffer_index() >= scene_.framebuffers_size())
            {
                VkClearValue clearValue;
                clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
                clearValues.push_back(clearValue);
                for (auto& command : subpass.commands())
                {
                    if (scene_.pipelines(scene_.materials(command.material_index()).pipeline_index()).depth_test_enable())
                    {
                        clearValue.depthStencil = { 1.0f, 0 };
                        clearValues.push_back(clearValue);
                    }
                }
            }
            else
            {
                auto& framebuffer = scene_.framebuffers(subpass.framebuffer_index());
                for (auto& target : framebuffer.color_attachments())
                {
                    VkClearValue clearValue;
                    clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
                    clearValues.push_back(clearValue);
                }
                if (framebuffer.has_depth_stencil_attachment())
                {
                    VkClearValue clearValue;
                    clearValue.depthStencil = { 1.0f, 0 };
                    clearValues.push_back(clearValue);
                }
            }
        }
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(renderer.commandBuffers[renderer.imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        //Automatic draw
        for (int i = 0; i < scene_.render_pass().sub_passes_size(); i++)
        {
            const auto& subpass = scene_.render_pass().sub_passes(i);

            const auto commandSize = subpass.commands_size();
            for (int j = 0; j < commandSize; j++)
            {
                const auto& command = subpass.commands(j);
                for (auto* script : scripts_)
                {
                    auto& drawCommand = static_cast<DrawCommand&>(GetDrawCommand(i, j));
                    drawCommand.PreDrawBind();
                    if (script != nullptr)
                    {
                        script->Draw(&drawCommand);
                    }
                }
            }

            for (int j = 0; j < subpass.commands_size(); j++)
            {
                const auto& command = subpass.commands(j);
                if (!command.automatic_draw())
                    continue;
                auto& drawCommand = static_cast<DrawCommand&>(GetDrawCommand(i, j));
                drawCommand.PreDrawBind();
                Draw(drawCommand);
            }
            if (i < scene_.render_pass().sub_passes_size() - 1)
            {
                vkCmdNextSubpass(renderer.commandBuffers[renderer.imageIndex], VK_SUBPASS_CONTENTS_INLINE);
            }
        }

        vkCmdEndRenderPass(renderer.commandBuffers[renderer.imageIndex]);
    }
    //Raytracing or compute pass
    int raytracingCommand = 0;
    for (int i = 0; i < scene_.render_pass().sub_passes_size(); i++)
    {
        const auto& subpass = scene_.render_pass().sub_passes(i);

        const auto commandSize = subpass.raytracing_commands_size();
        for(int raytracingCommandIndex = 0; raytracingCommandIndex < commandSize; raytracingCommandIndex++)
        {
            auto& command = raytracingCommands_[raytracingCommandIndex];
            command.Bind();
            for(auto* script: scripts_)
            {
                if(script == nullptr)
                    continue;
                script->Dispatch(&command);
            }
            command.Dispatch();
            raytracingCommand++;
        }
    }
    if (raytracingCommand > 0)
    {
        const auto windowSize = core::GetWindowSize();
        auto& commandBuffer = renderer.commandBuffers[renderer.imageIndex];
        
        /*
                    Copy ray tracing output to swap chain image
                */

                // Prepare current swap chain image as transfer destination
        TransitionImageLayout(swapchain.images[renderer.imageIndex],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,1,
            commandBuffer);

        // Prepare ray tracing output image as transfer source
        TransitionImageLayout(
            raytracingStorageImage_.image.image,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            1,1,
            commandBuffer);

        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.srcOffset = { 0, 0, 0 };
        copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.dstOffset = { 0, 0, 0 };
        copyRegion.extent = { windowSize.x, windowSize.y, 1 };
        vkCmdCopyImage(commandBuffer, raytracingStorageImage_.image.image, 
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.images[renderer.imageIndex],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        // Transition swap chain image back for presentation
        TransitionImageLayout(
            swapchain.images[renderer.imageIndex],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            1,1,commandBuffer);

        // Transition ray tracing output image back to general layout
        TransitionImageLayout(
            raytracingStorageImage_.image.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            1,1,commandBuffer);
    }
}

void Scene::Draw(core::DrawCommand& drawCommand, int instance)
{
    const auto& renderer = GetRenderer();

    const auto pipelineIndex = scene_.materials(drawCommand.GetMaterialIndex()).pipeline_index();
    auto& pipeline = pipelines_[pipelineIndex];
    pipeline.Bind();
    const auto meshIndex = drawCommand.GetMeshIndex();
    VkDeviceSize offsets[] = { 0 };
    if (meshIndex != -1 && scene_.meshes(meshIndex).primitve_type() != core::pb::Mesh_PrimitveType_NONE)
    {
        const auto& vertexBuffer = vertexBuffers_[meshIndex];
        vkCmdBindVertexBuffers(renderer.commandBuffers[renderer.imageIndex], 0, 1, &vertexBuffer.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(renderer.commandBuffers[renderer.imageIndex], vertexBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    }
    switch(drawCommand.GetInfo().mode())
    {
    case core::pb::DrawCommand_Mode_TRIANGLES:
        vkCmdSetPrimitiveTopology(renderer.commandBuffers[renderer.imageIndex],
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        break;
    case core::pb::DrawCommand_Mode_TRIANGLE_STRIP:
        vkCmdSetPrimitiveTopology(renderer.commandBuffers[renderer.imageIndex],
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        break;
    default: 
        break;
    }
    if (drawCommand.GetInfo().draw_elements())
    {
        vkCmdDrawIndexed(renderer.commandBuffers[renderer.imageIndex],
            drawCommand.GetInfo().count(),
            instance,
            0,
            0,
            0);
    }
    else
    {
        vkCmdDraw(renderer.commandBuffers[renderer.imageIndex],
            drawCommand.GetInfo().count(),
            instance, 
            0,
            0);
    }
}

Framebuffer& Scene::GetFramebuffer(int framebufferIndex)
{
    return framebuffers_[framebufferIndex];
}

Framebuffer& Scene::GetFramebuffer(std::string_view framebufferName)
{
    return *std::ranges::find_if(framebuffers_, [framebufferName](const auto& framebuffer)
        {
            return framebuffer.GetName() == framebufferName;
        });
}

core::SceneMaterial Scene::GetMaterial(int materialIndex)
{
    return {nullptr, nullptr};
}

Pipeline& Scene::GetPipeline(int index)
{
    return pipelines_[index];
}


VkRenderPass Scene::GetCurrentRenderPass() const
{
    return renderPass_.renderPass;
}

const Texture& Scene::GetTexture(int index) const
{
    const auto& textureManager = static_cast<TextureManager&>(core::GetTextureManager());
    return textureManager.GetTexture(textures_[index].textureId);
}

void Scene::OnEvent(SDL_Event& event)
{
    core::Scene::OnEvent(event);
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
    {
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
            ResizeWindow();
            break;
        default: break;
        }
        break;
    }
    default: break;
    }
    
}

Pipeline& Scene::GetRaytracingPipeline(int raytracingPipelineIndex)
{
    return raytracingPipelines_[raytracingPipelineIndex];
}

TopLevelAccelerationStructure& Scene::GetAccelerationStructure(int accelerationStructureIndex)
{
    return topLevelAccelerationStructures_[accelerationStructureIndex];
}

void Scene::Dispatch(core::ComputeCommand& command, int x, int y, int z)
{
    //TODO
}

Scene::ImportStatus Scene::LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb)
{
    LogDebug("Load Shaders");
    const auto& driver = GetDriver();
    const auto& filesystem = core::FilesystemLocator::get();
    for (auto& shaderPb : shadersPb)
    {
        const auto shaderFile = filesystem.LoadFile(shaderPb.path());
        auto shaderModule = CreateShaderModule(shaderFile, driver.device);
        if (!shaderModule)
            return ImportStatus::FAILURE;
        Shader shader{.module = shaderModule.value()};

        shader.stage = GetShaderStage(shaderPb.type());
        shaders_.push_back(shader);
        
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadPipelines(
    const PbRepeatField<core::pb::Pipeline>& pipelines,
    const PbRepeatField<core::pb::RaytracingPipeline>& raytracingPipelines)
{
    LogDebug("Load Pipelines");
    auto getShader = [this](const int index, core::pb::ShaderType type)->std::optional<std::reference_wrapper<Shader>>
    {
        if (index != -1 && scene_.shaders(index).type() == type)
        {
            return shaders_[index];
        }
        return std::nullopt;
    };
    pipelines_.resize(pipelines.size());
    for(int i = 0; i < pipelines.size(); i++)
    {
        Pipeline& pipeline = pipelines_[i];
        const auto& pipelinePb = pipelines[i];
        
        switch(pipelinePb.type())
        {
        case core::pb::Pipeline_Type_RASTERIZE:
        {
            if (!pipeline.LoadRasterizePipeline(pipelinePb,
                shaders_[pipelinePb.vertex_shader_index()],
                shaders_[pipelinePb.fragment_shader_index()],
                i,
                getShader(pipelinePb.geometry_shader_index(), core::pb::GEOMETRY),
                getShader(pipelinePb.tess_control_shader_index(), core::pb::TESSELATION_CONTROL),
                getShader(pipelinePb.tess_eval_shader_index(), core::pb::TESSELATION_EVAL)))
            {
                return ImportStatus::FAILURE;
            }
            break;
        }
        case core::pb::Pipeline_Type_COMPUTE:
        {
            break;
        }
        default: break;
        }
    }

    LogDebug("Load Raytracing Pipelines");
    raytracingPipelines_.resize(raytracingPipelines.size());
    for(int i = 0; i < raytracingPipelines.size(); i++)
    {

        Pipeline& pipeline = raytracingPipelines_[i];
        const auto& raytracingPipelinePb = scene_.raytracing_pipelines(i);
        if (!pipeline.LoadRaytracingPipeline(raytracingPipelinePb,
            shaders_[raytracingPipelinePb.ray_gen_shader_index()],
            shaders_[raytracingPipelinePb.miss_hit_shader_index()],
            shaders_[raytracingPipelinePb.closest_hit_shader_index()],
            getShader(raytracingPipelinePb.any_hit_shader_index(), core::pb::RAY_ANY_HIT),
            getShader(raytracingPipelinePb.intersection_hit_shader_index(), core::pb::RAY_INTERSECTION)))
        {
            return ImportStatus::FAILURE;
        }
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<core::pb::Texture>& textures)
{
    LogDebug("Load Texture");
    const auto texturesSize = textures.size();
    textures_.resize(texturesSize);
    auto& textureManager = core::GetTextureManager();
    for (int i = 0; i < texturesSize; i++)
    {
        textures_[i] = { textureManager.LoadTexture(scene_.textures(i)) };
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadMaterials(const PbRepeatField<core::pb::Material>& materials)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadModels(const PbRepeatField<std::string>& models)
{
    auto& modelManager = core::GetModelManager();
    const auto modelsSize = models.size();
    for (int i = 0; i < modelsSize; i++)
    {
        modelIndices_.push_back(modelManager.ImportModel(models.Get(i)));
    }

    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes)
{
    LogDebug("Load Meshes");
    auto& modelManager = core::GetModelManager();
    const auto meshesSize = meshes.size();
    for (int i = 0; i < meshesSize; i++)
    {
        const auto& meshInfo = meshes.Get(i);
        switch (meshInfo.primitve_type())
        {
        case core::pb::Mesh_PrimitveType_QUAD:
        {
            const glm::vec3 scale = meshInfo.has_scale() ? glm::vec3{ meshInfo.scale().x(), meshInfo.scale().y(), meshInfo.scale().z() } : glm::vec3(1.0f);
            const glm::vec3 offset{ meshInfo.offset().x(), meshInfo.offset().y(), meshInfo.offset().z() };
            const auto mesh = core::GenerateQuad(scale, offset);
            vertexBuffers_.emplace_back();
            
            vertexBuffers_.back() = CreateVertexBufferFromMesh(mesh);
            break;
        }
        case core::pb::Mesh_PrimitveType_CUBE:
        {
            const glm::vec3 scale = meshInfo.has_scale() ? glm::vec3{ meshInfo.scale().x(), meshInfo.scale().y(), meshInfo.scale().z() } : glm::vec3(1.0f);
            const glm::vec3 offset{ meshInfo.offset().x(), meshInfo.offset().y(), meshInfo.offset().z() };

            const auto mesh = core::GenerateCube(scale, offset);
            vertexBuffers_.emplace_back();
            vertexBuffers_.back() = CreateVertexBufferFromMesh(mesh);;
            break;
        }
        case core::pb::Mesh_PrimitveType_SPHERE:
        {
            const glm::vec3 scale = meshInfo.has_scale() ? glm::vec3{ meshInfo.scale().x(), meshInfo.scale().y(), meshInfo.scale().z() } : glm::vec3(1.0f);
            const glm::vec3 offset{ meshInfo.offset().x(), meshInfo.offset().y(), meshInfo.offset().z() };

            const auto mesh = core::GenerateSphere(scale.x, offset);
            vertexBuffers_.emplace_back();
            vertexBuffers_.back() = CreateVertexBufferFromMesh(mesh);

            break;
        }
        case core::pb::Mesh_PrimitveType_NONE:
        {
            vertexBuffers_.emplace_back();
            break;
        }
        case core::pb::Mesh_PrimitveType_MODEL:
        {
            const auto& mesh = modelManager.GetModel(modelIndices_[meshInfo.model_index()]).GetMesh(meshInfo.mesh_name());
            vertexBuffers_.emplace_back();
            vertexBuffers_.back() = CreateVertexBufferFromMesh(mesh);
            break;
        }
        default:
            break;
        }
    }
    const auto& topLevelAccelerationStructures = scene_.top_level_acceleration_structures();
    topLevelAccelerationStructures_.resize(topLevelAccelerationStructures.size());
    for(int i = 0; i < topLevelAccelerationStructures.size(); i++)
    {
        topLevelAccelerationStructures_[i].Create(topLevelAccelerationStructures.Get(i));
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers)
{
    LogDebug("Load Framebuffers");
    for(const auto& framebufferInfo : framebuffers)
    {
        framebuffers_.emplace_back();
        auto& framebuffer = framebuffers_.back();
        framebuffer.Load(framebufferInfo);
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadRenderPass(const core::pb::RenderPass& renderPassPb)
{
    LogDebug("Creating Render Pass");
    auto& driver = GetDriver();
    auto& swapchain = GetSwapchain();

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkImageView> imageViews;
    
    //Import all subpasses with their attachments refs and dependency
    struct SubpassData
    {
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        std::vector<VkAttachmentReference> inputReferences;
        VkAttachmentReference depthAttachmentRef;
        bool isBackBuffer = false;
        bool hasDepth = false;
        bool isRaytracing = false;
    };
    struct RenderTargetData
    {
        std::string framebufferName;
        std::string attachmentName;
        int attachmentIndex = -1;
    };
    std::vector<VkSubpassDescription> subpasses(renderPassPb.sub_passes_size());
    std::vector<SubpassData> subpassDatas(renderPassPb.sub_passes_size());
    std::vector<RenderTargetData> renderTargetDatas;
    int raytracingPass = 0;
    int rasterizePass = 0;
    int computePass = 0;
    for (int i = 0; i < renderPassPb.sub_passes_size(); i++)
    {
        const auto& subpassPb = renderPassPb.sub_passes(i);

        auto& colorAttachmentRefs = subpassDatas[i].colorAttachmentRefs;
        const auto framebufferIndex = subpassPb.framebuffer_index();
        if(framebufferIndex < 0 || framebufferIndex >= static_cast<int>(framebuffers_.size()))
        {
            //Backbuffer
            subpassDatas[i].isBackBuffer = true;

            bool hasDepth = false;
            for (auto& command : subpassPb.commands())
            {
                if (scene_.pipelines(scene_.materials(command.material_index()).pipeline_index()).depth_test_enable())
                {
                    hasDepth = true;
                    break;
                }
            }
            int attachmentIndex = attachments.size();
            attachments.emplace_back();
            auto& attachmentDescription = attachments.back();
            attachmentDescription.format = swapchain.imageFormat;
            attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            colorAttachmentRefs.resize(1);
            auto& colorAttachementRef = colorAttachmentRefs[0];
            colorAttachementRef.attachment = attachmentIndex;
            colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            subpasses[i].colorAttachmentCount = colorAttachmentRefs.size();
            subpasses[i].pColorAttachments = colorAttachmentRefs.data();
            if (hasDepth)
            {
                subpassDatas[i].hasDepth = true;
                attachmentIndex = attachments.size();
                attachments.emplace_back();
                auto& attachmentDescription = attachments.back();
                attachmentDescription.format = FindDepthFormat(driver.physicalDevice);
                attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
                attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                
                auto& depthAttachmentRef = subpassDatas[i].depthAttachmentRef;
                depthAttachmentRef.attachment = attachmentIndex;
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpasses[i].pDepthStencilAttachment = &subpassDatas[i].depthAttachmentRef;
            }

        }
        else
        {
            
            //other "framebuffer"
            auto& framebufferPb = scene_.framebuffers(subpassPb.framebuffer_index());

            colorAttachmentRefs.resize(framebufferPb.color_attachments_size());

            bool hasDepth = framebufferPb.has_depth_stencil_attachment();

            for (int j = 0; j < framebufferPb.color_attachments_size(); j++)
            {
                const auto& attachmentRefPb = framebufferPb.color_attachments(j);

                const auto attachmentIndex = attachments.size();
                attachments.emplace_back();
                renderTargetDatas.push_back({framebufferPb.name(),
                                             attachmentRefPb.name(),
                                             static_cast<int>(attachmentIndex)});


                auto& attachmentDescription = attachments.back();
                auto& renderTarget = GetFramebuffer(framebufferIndex)
                    .GetRenderTarget(attachmentRefPb.name());
                attachmentDescription.format = renderTarget.format;
                attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
                attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                imageViews.push_back(renderTarget.imageView);

                auto& colorAttachementRef = colorAttachmentRefs[j];
                colorAttachementRef.attachment = attachmentIndex;
                colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            }


            subpasses[i].colorAttachmentCount = colorAttachmentRefs.size();
            subpasses[i].pColorAttachments = colorAttachmentRefs.data();

            if (hasDepth)
            {
                auto& renderTarget = GetFramebuffer(framebufferIndex)
                    .GetRenderTarget(framebufferPb.depth_stencil_attachment().name());
                const auto attachmentIndex = attachments.size(); 
                attachments.emplace_back();
                renderTargetDatas.push_back({framebufferPb.name(),
                                             framebufferPb.depth_stencil_attachment().name(),
                                             static_cast<int>(attachmentIndex)});

                auto& attachmentDescription = attachments.back();
                attachmentDescription.format = renderTarget.format;
                attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
                attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                imageViews.push_back(renderTarget.imageView);

                auto& depthAttachmentRef = subpassDatas[i].depthAttachmentRef;
                depthAttachmentRef.attachment = attachmentIndex; 
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpasses[i].pDepthStencilAttachment = &subpassDatas[i].depthAttachmentRef;
            }

        }
        std::vector<int> inputAttachmentIndices;
        for(auto& command: subpassPb.commands())
        {
            for(auto& materialTexture: scene_.materials(command.material_index()).textures())
            {
                if(materialTexture.framebuffer_name().empty())
                    continue;
                auto it = std::ranges::find_if(renderTargetDatas, 
                    [&materialTexture](const auto& renderTargetData)
                    {
                        return renderTargetData.framebufferName == materialTexture.framebuffer_name() &&
                            renderTargetData.attachmentName == materialTexture.attachment_name();
                    });
                if(std::ranges::none_of(inputAttachmentIndices, [&it](auto inputAttachment)
                {
                        return it->attachmentIndex == inputAttachment;
                }))
                {
                    inputAttachmentIndices.push_back(it->attachmentIndex);
                }
            }
        }

        if (subpassPb.commands_size() > 0)
        {
            rasterizePass++;
        }

        for(auto& command : subpassPb.raytracing_commands())
        {
            subpassDatas[i].isRaytracing = true;
            //TODO link other framebuffer attachment
        }
        if(subpassDatas[i].isRaytracing)
        {
            raytracingPass++;
        }
        subpassDatas[i].inputReferences.reserve(inputAttachmentIndices.size());
        for(auto& inputAtt : inputAttachmentIndices)
        {
            subpassDatas[i].inputReferences.push_back({static_cast<std::uint32_t>(inputAtt), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
        }
        subpasses[i].inputAttachmentCount = subpassDatas[i].inputReferences.size();
        subpasses[i].pInputAttachments = subpassDatas[i].inputReferences.data();

    }
    if(raytracingPass > 0 && rasterizePass == 0)
    {
        //Pure raytracing scene
        LogDebug("Create Raytracing Storage Image for pure raytracing");
        auto& swapchain = GetSwapchain();
        const auto windowSize = core::GetWindowSize();
        raytracingStorageImage_.image = CreateImage(windowSize.x,
            windowSize.y,
            swapchain.imageFormat,
            1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            1);
        TransitionImageLayout(raytracingStorageImage_.image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 1, 1);
        raytracingStorageImage_.imageView = CreateImageView(driver.device, raytracingStorageImage_.image.image, swapchain.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        return ImportStatus::SUCCESS;
    }
    std::vector<VkSubpassDependency> dependencies;
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies.push_back(dependency);
    }
    for(int i = 1; i < renderPassPb.sub_passes_size(); i++)
    {
        //Waiting for the previous subpass to finish
        VkSubpassDependency dependency{};
        dependency.srcSubpass = i-1;
        dependency.dstSubpass = i;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //TODO add compute if subpass is compute or raytrace, and geometry if enable and tesselation if enable
        dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies.push_back(dependency);
    }

    // Generate the renderpass
    VkRenderPass renderPass;
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (const auto result = vkCreateRenderPass(driver.device, &renderPassInfo, nullptr, &renderPass); result != VK_SUCCESS)
    {
        LogError("Failed to create render pass!");
        return ImportStatus::FAILURE;
    }
    renderPass_.renderPass = renderPass;
    auto& renderer = GetRenderer();
    renderPass_.framebuffers.resize(swapchain.imageViews.size());
    for (size_t i = 0; i < swapchain.imageViews.size(); i++)
    {
        auto imageViewsTmp = imageViews;
        imageViewsTmp.push_back(swapchain.imageViews[i]);
        if(subpassDatas.back().hasDepth)
        {
            imageViewsTmp.push_back(swapchain.depthImageView);
        }
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = imageViewsTmp.size();
        framebufferInfo.pAttachments = imageViewsTmp.data();
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(driver.device, &framebufferInfo, nullptr,
            &renderPass_.framebuffers[i]) != VK_SUCCESS)
        {
            LogError("Failed to create framebuffer!");
            return ImportStatus::FAILURE;
        }
    }
    return ImportStatus::SUCCESS;
}

core::DrawCommand& Scene::GetDrawCommand(int subPassIndex, int drawCommandIndex) {
    int resultIndex = 0;
    for(int i = 0; i < this->scene_.render_pass().sub_passes_size(); i++)
    {
        if(i < subPassIndex)
        {
            resultIndex += scene_.render_pass().sub_passes(i).commands_size();
        }
        else if(i == subPassIndex)
        {
            break;

        }
    }
    return drawCommands_[resultIndex + drawCommandIndex];
}

Scene::ImportStatus Scene::LoadDrawCommands(const core::pb::RenderPass& renderPass)
{
    LogDebug("Loading Draw Commands");
    for (int i = 0; i < renderPass.sub_passes_size(); i++)
    {
        const auto& subpassPb = renderPass.sub_passes(i);
        for(int j = 0; j < subpassPb.commands_size(); j++)
        {
            drawCommands_.emplace_back(subpassPb.commands(j), i);
            drawCommands_.back().Create();
        }
        for(int j = 0; j < subpassPb.raytracing_commands_size(); j++)
        {
            raytracingCommands_.emplace_back(subpassPb.raytracing_commands(j));
            raytracingCommands_.back().Create();
        }
    }
    return Scene::ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadBuffers(const PbRepeatField<core::pb::Buffer>& buffers)
{
    return ImportStatus::FAILURE;
}

void Scene::ResizeWindow()
{
    auto& driver = GetDriver();
    //Destroy resize dependent objects
    for (auto& framebuffer : framebuffers_)
    {
        framebuffer.Destroy();
    }
    framebuffers_.clear();
    for (const auto& framebuffer : renderPass_.framebuffers)
    {
        vkDestroyFramebuffer(driver.device, framebuffer, nullptr);
    }
    renderPass_.framebuffers.clear();
    for (auto& drawCommand : drawCommands_)
    {
        drawCommand.Destroy();
    }
    drawCommands_.clear();
    for(auto& command: raytracingCommands_)
    {
        command.Destroy();
    }
    raytracingCommands_.clear();
    for (auto& pipeline : pipelines_)
    {
        pipeline.Destroy();
    }
    pipelines_.clear();
    vkDestroyRenderPass(driver.device, renderPass_.renderPass, nullptr);
    renderPass_.renderPass = VK_NULL_HANDLE;

    //Recreate them
    const auto& framebuffers = scene_.framebuffers();
    if (LoadFramebuffers(framebuffers) != ImportStatus::SUCCESS)
    {
        LogError("Could not recreate framebuffers");
    }
    const auto& renderPass = scene_.render_pass();
    if (LoadRenderPass(renderPass) != ImportStatus::SUCCESS)
    {
        LogError("Count not recreate render pass");
    }
    const auto& pipelines = scene_.pipelines();
    const auto& raytracingPipelines = scene_.raytracing_pipelines();
    if (LoadPipelines(pipelines, raytracingPipelines) != ImportStatus::SUCCESS)
    {
        LogError("Could not recreate pipelines");
    }

    if (LoadDrawCommands(renderPass) != ImportStatus::SUCCESS)
    {
        LogError("Could not recreate draw commands");
    }
}

VkRenderPass GetCurrentRenderPass()
{
    const auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    return scene->GetCurrentRenderPass();
}

VkCommandBuffer GetCurrentCommandBuffer()
{
    auto& renderer = GetRenderer();
    return renderer.commandBuffers[renderer.imageIndex];
}

VkPipelineLayout GetPipelineLayout(int pipelineIndex, int raytracingPipelineIndex)
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    if (pipelineIndex == -1 && raytracingPipelineIndex == -1)
    {
        return VK_NULL_HANDLE;
    }
    else if (pipelineIndex != -1)
    {
        return scene->GetPipeline(pipelineIndex).GetLayout();
    }
    return scene->GetRaytracingPipeline(raytracingPipelineIndex).GetLayout();
}

VkDescriptorSetLayout GetDescriptorSetLayout(int pipelineIndex, int raytracingPipelineIndex)
{
    auto* scene = static_cast<Scene*>(core::GetCurrentScene());
    if(pipelineIndex == -1 && raytracingPipelineIndex == -1)
    {
        return VK_NULL_HANDLE;
    }
    else if(pipelineIndex != -1)
    {
        return scene->GetPipeline(pipelineIndex).GetDescriptorSetLayout();
    }
    return scene->GetRaytracingPipeline(raytracingPipelineIndex).GetDescriptorSetLayout();
}
}
