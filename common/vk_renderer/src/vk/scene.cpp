#include "vk/scene.h"

#include "vk/engine.h"
#include "vk/utils.h"
#include "vk/window.h"
#include "vk/pipeline.h"

#include "renderer/draw_command.h"

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
    for (const auto& framebuffer : vkFramebuffers_)
    {
        vkDestroyFramebuffer(driver.device, framebuffer, nullptr);
    }
    vkFramebuffers_.clear();
    for(auto& pipeline : pipelines_)
    {
        pipeline.Destroy();
    }
    pipelines_.clear();
    vkDestroyRenderPass(driver.device, renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;
    auto& textureManager = core::GetTextureManager();
    textureManager.Clear();
    auto& allocator = GetAllocator();
    for(auto& vertexBuffer: vertexBuffers_)
    {
        vmaDestroyBuffer(allocator, vertexBuffer.vertexBuffer.buffer, vertexBuffer.vertexBuffer.allocation);
        vmaDestroyBuffer(allocator, vertexBuffer.indexBuffer.buffer, vertexBuffer.indexBuffer.allocation);
    }
    vertexBuffers_.clear();
}

void Scene::Update(float dt)
{
    auto& renderer = GetRenderer();
    auto& swapchain = GetSwapchain();
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass_;
    renderPassInfo.framebuffer = vkFramebuffers_[renderer.imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchain.extent;

    //Fill with subpass clear values
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(renderer.commandBuffers[renderer.imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Automatic draw
    for(int i = 0; i < scene_.render_pass().sub_passes_size(); i++)
    {
        const auto& subpass = scene_.render_pass().sub_passes(i);
        
        const auto commandSize = subpass.commands_size();
        for (int j = 0; j < commandSize; j++)
        {
            const auto& command = subpass.commands(j);
            for (auto* pySystem : pySystems_)
            {
                if (pySystem != nullptr)
                {
                    pySystem->Draw(&GetDrawCommand(i, j));
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
    }

    vkCmdEndRenderPass(renderer.commandBuffers[renderer.imageIndex]);
}

void Scene::Draw(core::DrawCommand& drawCommand)
{
    const auto& renderer = GetRenderer();

    const auto pipelineIndex = scene_.materials(drawCommand.GetMaterialIndex()).pipeline_index();
    auto& pipeline = pipelines_[pipelineIndex];
    pipeline.Bind();
    const auto meshIndex = drawCommand.GetMeshIndex();
    VkDeviceSize offsets[] = { 0 };
    if (meshIndex != -1)
    {
        const auto& vertexBuffer = vertexBuffers_[meshIndex];
        vkCmdBindVertexBuffers(renderer.commandBuffers[renderer.imageIndex], 0, 1, &vertexBuffer.vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(renderer.commandBuffers[renderer.imageIndex], vertexBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    }
    if (drawCommand.GetInfo().draw_elements())
    {
        vkCmdDrawIndexed(renderer.commandBuffers[renderer.imageIndex],
            drawCommand.GetInfo().count(),
            1,
            0,
            0,
            0);
    }
    else
    {
        vkCmdDraw(renderer.commandBuffers[renderer.imageIndex],
            drawCommand.GetInfo().count(),
            1, 
            0,
            0);
    }
}

Framebuffer& Scene::GetFramebuffer(int framebufferIndex)
{
    return framebuffers_[framebufferIndex];
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
    return renderPass_;
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

Scene::ImportStatus Scene::LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines)
{
    LogDebug("Load Pipelines");
    pipelines_.resize(pipelines.size());
    for(int i = 0; i < pipelines.size(); i++)
    {
        Pipeline& pipeline = pipelines_[i];
        const auto& pipelinePb = pipelines[i];
        if(!pipeline.LoadRaterizePipeline(pipelinePb, shaders_[pipelinePb.vertex_shader_index()], shaders_[pipelinePb.fragment_shader_index()]))
        {
            return ImportStatus::FAILURE;
        }
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<core::pb::Texture>& textures)
{
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
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes)
{
    LogDebug("Load Meshes");
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
            break;
        case core::pb::Mesh_PrimitveType_NONE:
        {
            vertexBuffers_.emplace_back();
            //vertexBuffers_.back() = CreateVertexBufferFromMesh(mesh);;
            break;
        }
        case core::pb::Mesh_PrimitveType_MODEL:
        {
            break;
        }
        default:
            break;
        }
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers)
{
    //TODO needs to gather all additional attachments for the render pass generation and the vkframebuffer 

    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadRenderPass(const core::pb::RenderPass& renderPassPb)
{
    LogDebug("Creating Render Pass");
    auto& driver = GetDriver();
    auto& swapchain = GetSwapchain();

    std::vector<VkAttachmentDescription> attachments(1);
    //TODO add the attachments from the framebuffers and the depth stencil
    //Adding the present attachment
    {
        auto& attachmentDescription = attachments.back();
        attachmentDescription.format = swapchain.imageFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    
    //Import all subpasses with their attachments refs and dependency

    struct SubpassData
    {
        std::vector<VkAttachmentReference> colorAttachmentRefs;
        VkAttachmentReference depthAttachmentRef;
    };
    std::vector<VkSubpassDescription> subpasses(renderPassPb.sub_passes_size());
    std::vector<SubpassData> subpassDatas(renderPassPb.sub_passes_size());
    bool hasRenderPassDepth = false;
    for (int i = 0; i < renderPassPb.sub_passes_size(); i++)
    {
        const auto& subpassPb = renderPassPb.sub_passes(i);

        auto& colorAttachmentRefs = subpassDatas[i].colorAttachmentRefs;
        const auto framebufferIndex = subpassPb.framebuffer_index();
        if(framebufferIndex < 0 || framebufferIndex >= static_cast<int>(framebuffers_.size()))
        {
            colorAttachmentRefs.resize(1);
            auto& colorAttachementRef = colorAttachmentRefs[0];
            const auto attachmentIndex = attachments.size()-1; //FIXME color present attachment
            colorAttachementRef.attachment = attachmentIndex;
            colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            subpasses[i].colorAttachmentCount = colorAttachmentRefs.size();
            subpasses[i].pColorAttachments = colorAttachmentRefs.data();
        }
        else
        {
            auto& framebufferPb = scene_.framebuffers(subpassPb.framebuffer_index());

            colorAttachmentRefs.resize(framebufferPb.color_attachments_size());


            for (int j = 0; j < colorAttachmentRefs.size(); j++)
            {
                const auto& attachmentRefPb = framebufferPb.color_attachments(j);
                auto& colorAttachementRef = colorAttachmentRefs[j];
                const auto attachmentIndex = j; //FIXME global attachment index
                colorAttachementRef.attachment = attachmentIndex;
                colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            }
            subpasses[i].colorAttachmentCount = colorAttachmentRefs.size();
            subpasses[i].pColorAttachments = colorAttachmentRefs.data();

            if (framebufferPb.has_depth_stencil_attachment())
            {
                hasRenderPassDepth = true;
                auto& depthAttachmentRef = subpassDatas[i].depthAttachmentRef;
                depthAttachmentRef.attachment = colorAttachmentRefs.size(); //FIXME global attachment index with color and depth combined
                depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                subpasses[i].pDepthStencilAttachment = &subpassDatas[i].depthAttachmentRef;
            }
        }

    }


    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    // Generate the renderpass
    VkRenderPass renderPass;
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();
    //TODO use proper dependencies
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (const auto result = vkCreateRenderPass(driver.device, &renderPassInfo, nullptr, &renderPass); result != VK_SUCCESS)
    {
        LogError("Failed to create render pass!");
        return ImportStatus::FAILURE;
    }
    renderPass_ = renderPass;
    //TODO Generate the vkframebuffer
    auto& renderer = GetRenderer();
    vkFramebuffers_.resize(swapchain.imageViews.size());
    for (size_t i = 0; i < swapchain.imageViews.size(); i++)
    {
        //TODO add the other color attachments
        std::array<VkImageView, 2> attachments = {
            swapchain.imageViews[i],
            renderer.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = hasRenderPassDepth ?
            static_cast<uint32_t>(attachments.size()) :
            1;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(driver.device, &framebufferInfo, nullptr,
            &vkFramebuffers_[i]) != VK_SUCCESS)
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
    for (int i = 0; i < renderPass.sub_passes_size(); i++)
    {
        const auto& subpassPb = renderPass.sub_passes(i);
        for(int j = 0; j < subpassPb.commands_size(); j++)
        {
            drawCommands_.emplace_back(subpassPb.commands(j), i);
            drawCommands_.back().GenerateUniforms();
        }
    }
    return Scene::ImportStatus::SUCCESS;
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
}
