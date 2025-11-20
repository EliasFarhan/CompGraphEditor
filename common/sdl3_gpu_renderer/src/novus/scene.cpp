#include "novus/texture.h"
#include "novus/scene.h"

#include "novus/render_pass.h"

namespace novus
{
void Scene::UnloadScene()
{
    for (auto& pipeline : pipelines_)
    {
        pipeline.Destroy();
    }
    for (auto& shader : shaders_)
    {
        shader.Destroy();
    }
    for (auto& vertexInputBuffer : vertexInputBuffers_)
    {
        vertexInputBuffer.Destroy();
    }
    GetTextureManager().Clear();
    bufferManager_.Clear();
    if (depthTexture_ != nullptr)
    {
        SDL_ReleaseGPUTexture(GetDevice(), depthTexture_);
    }
}

void Scene::Update(float dt)
{
    auto* commandBuffer = GetCommandBuffer();

    for (auto& script: scripts_)
    {
        if (script != nullptr)
        {
            script->Update(dt);
        }
    }
    bufferManager_.UploadStorageBuffers(commandBuffer);
    SDL_GPUTexture* swapchainTexture = GetSwapchainTexture();
    if (swapchainTexture != nullptr)
    {
        std::array backBufferTarget = {swapchainTexture};

        for (int64_t subpassIndex = 0; subpassIndex < std::ssize(renderpasses_); subpassIndex++)
        {
            auto& subpass = renderpasses_[subpassIndex];
            const auto& subpassInfo = scene_.sub_passes[subpassIndex];
            const auto framebufferIndex = subpassInfo.framebuffer_index;
            //TODO define framebuffer to get the needed textures (like in OpenGL?)
            currentRenderPass_ = GenerateSubPass(commandBuffer, subpassInfo, backBufferTarget, depthTexture_,
                framebufferIndex == -1 ? backBufferInfo_ : scene_.framebuffer[framebufferIndex]);
            for (auto& command: subpass.GetDrawCommands())
            {
                command.Bind(currentRenderPass_);

                if (command.GetMeshIndex() != -1)
                {
                    auto& vertexInputBuffer = vertexInputBuffers_[command.GetMeshIndex()];
                    vertexInputBuffer.Bind(currentRenderPass_);
                }

                for (auto& script: scripts_)
                {
                    if (script != nullptr)
                    {
                        script->Draw(&command);
                    }
                }

                if (command.GetDrawCommandInfo().automatic_draw)
                {
                    command.PreDrawBind(currentRenderPass_);
                    if (command.GetDrawCommandInfo().draw_elements)
                    {
                        SDL_DrawGPUIndexedPrimitives(currentRenderPass_, command.GetDrawCommandInfo().count, 1, 0, 0, 0);
                    }
                    else
                    {
                        SDL_DrawGPUPrimitives(currentRenderPass_, command.GetDrawCommandInfo().count, 1, 0, 0);
                    }
                }
            }
            SDL_EndGPURenderPass(currentRenderPass_);
            currentRenderPass_ = nullptr;
        }
    }
}

void Scene::Draw(core::DrawCommand& drawCommand, int instance)
{
    const auto& drawCommandInfo = drawCommand.GetDrawCommandInfo();
    if (drawCommandInfo.draw_elements)
    {
        SDL_DrawGPUIndexedPrimitives(currentRenderPass_, drawCommandInfo.count, instance, 0, 0, 0);
    }
    else
    {
        SDL_DrawGPUPrimitives(currentRenderPass_, drawCommandInfo.count, instance, 0, 0);
    }
}

void Scene::Dispatch(core::ComputeCommand& command, int x, int y, int z)
{
}

core::Framebuffer& Scene::GetFramebuffer(int framebufferIndex)
{
    return framebuffers_[framebufferIndex];
}

core::SceneMaterial Scene::GetMaterial(int materialIndex)
{
    auto& material = materials_[materialIndex];
    return {&pipelines_[material.GetPipelineIndex()], &material};
}

core::Pipeline& Scene::GetPipeline(int index)
{
    return pipelines_[index];
}

core::DrawCommand& Scene::GetDrawCommand(int subPassIndex, int drawCommandIndex)
{
    return renderpasses_.at(subPassIndex).GetDrawCommands()[drawCommandIndex];
}

Scene::ImportStatus Scene::LoadTextures(std::span<const renderer::TextureT> textures)
{

    auto& textureManager = GetTextureManager();
    textures_.resize(textures.size());
    for (int64_t i = 0; i < std::ssize(textures); ++i)
    {
        auto& textureInfo = textures[i];
        textures_[i] = textureManager.LoadTexture(textureInfo);
    }

    textureManager.UploadTextures();

    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadShaders(std::span<const renderer::ShaderT> shadersPb)
{
    shaders_.resize(shadersPb.size());
    for (size_t i = 0; i < shadersPb.size(); ++i)
    {
        shaders_[i].LoadShader(shadersPb[i]);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadPipelines(std::span<const renderer::GraphicsPipelineT> pipelineInfos) {
    pipelines_.resize(pipelineInfos.size());
    for (size_t pipelineIndex = 0; pipelineIndex < pipelines_.size(); ++pipelineIndex)
    {
        auto& pipelineInfo = pipelineInfos[pipelineIndex];
        Pipeline& pipeline = pipelines_[pipelineIndex];
        pipeline.Load(pipelineInfos[pipelineIndex],
            shaders_[pipelineInfo.vertex_shader_index],
            scene_.shaders[pipelineInfo.vertex_shader_index],
            shaders_[pipelineInfo.fragment_shader_index],
            scene_.shaders[pipelineInfo.fragment_shader_index]);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadMaterials(std::span<const renderer::MaterialT> materials)
{
    materials_.reserve(materials.size());
    for (const auto & materialInfo : materials)
    {
        const auto pipelineIndex = materialInfo.pipeline_index;
        Material material(&pipelines_[pipelineIndex]);
        material.Load(materialInfo, bufferManager_, textures_);
        materials_.push_back(std::move(material));
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadMeshes(std::span<const renderer::MeshT> meshes)
{
    auto* commandBuffer = SDL_AcquireGPUCommandBuffer(GetDevice());
    auto* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    vertexInputBuffers_.resize(meshes.size());
    for (size_t i = 0; i < meshes.size(); ++i)
    {
        const auto& meshInfo = meshes[i];
        if (meshInfo.primitive_type == renderer::MeshPrimitiveType_NONE)
            continue;

        auto& vertexInputBuffer = vertexInputBuffers_[i];
        switch (meshInfo.primitive_type)
        {
        case renderer::MeshPrimitiveType_CUBE:
        {
            auto mesh = core::GenerateCube(glm::vec3(1.0f), glm::vec3(0.0f));
            vertexInputBuffer.CreateFromMesh(mesh, copyPass);
            break;
        }
        case renderer::MeshPrimitiveType_QUAD:
        {
            auto mesh = core::GenerateQuad(glm::vec3(1.0f), glm::vec3(0.0f));
            vertexInputBuffer.CreateFromMesh(mesh, copyPass);
            break;
        }
        case renderer::MeshPrimitiveType_SPHERE:
        {
            auto mesh = core::GenerateSphere(1.0f, glm::vec3(0.0f));
            vertexInputBuffer.CreateFromMesh(mesh, copyPass);
            break;
        }
        default:
            break;
        }
    }
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    for (auto& vertexInputBuffer : vertexInputBuffers_)
    {
        vertexInputBuffer.ClearTransferBuffers();
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadRenderPass(std::span<const renderer::RenderpassT> renderPass)
{
    renderpasses_.reserve(renderPass.size());
    for (int64_t subpassIndex = 0; subpassIndex < std::ssize(renderPass); ++subpassIndex)
    {
        auto& subpass = renderPass[subpassIndex];

        if (subpass.framebuffer_index == -1) //means we use the backbuffer
        {
            if (depthTexture_ == nullptr)
            {
                bool generateDepthTexture = false;
                for (auto& command: subpass.commands)
                {
                    const auto& material = scene_.materials[command.material_index];
                    const auto& pipeline = scene_.pipelines[material.pipeline_index];

                    if (pipeline.info != nullptr && pipeline.info->depth_stencil_state != nullptr &&
                        pipeline.info->depth_stencil_state->enable_depth_test)
                    {
                        generateDepthTexture = true;
                    }
                }
                if (generateDepthTexture)
                {
                    auto windowSize = core::GetWindowSize();
                    SDL_GPUTextureCreateInfo textureCreateInfo{
                        .type = SDL_GPU_TEXTURETYPE_2D,
                        .format = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
                        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                        .width = windowSize.x, .height = windowSize.y,
                        .layer_count_or_depth = 1, .num_levels = 1, .sample_count = SDL_GPU_SAMPLECOUNT_1};
                    depthTexture_ = SDL_CreateGPUTexture(GetDevice(), &textureCreateInfo);
                    //TODO generate framebuffer data
                    auto depthStencilTargetInfo = std::make_unique<novus::internal::DepthStencilTargetInfoT>();
                    depthStencilTargetInfo->clear_depth = 1.0f;
                    depthStencilTargetInfo->clear_stencil = 0;
                    depthStencilTargetInfo->load_op = internal::LoadOp_LOADOP_CLEAR;
                    depthStencilTargetInfo->store_op = internal::StoreOp_STOREOP_STORE;
                    depthStencilTargetInfo->stencil_load_op = internal::LoadOp_LOADOP_CLEAR;
                    depthStencilTargetInfo->stencil_store_op = internal::StoreOp_STOREOP_STORE;
                    backBufferInfo_.depth_stencil_target_info = std::move(depthStencilTargetInfo);
                }
            }

        }
        //TODO generate depth buffer if available, what about framebuffer?
        renderpasses_.emplace_back(subpass, static_cast<int>(subpassIndex), pipelines_, materials_);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadBuffers(std::span<const renderer::StorageBufferT> buffers)
{
    for (const auto& storageBufferInfo : buffers)
    {
        bufferManager_.CreateBuffer(storageBufferInfo.name,
            storageBufferInfo.block_size, 1);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadFramebuffers(std::span<const renderer::FramebufferT> framebuffers)
{
    internal::ColorTargetInfoT colorTargetInfo{.mip_level = 0,
        .layer_or_depth_plane = 0,
        .clear_color = {.r = 0, .g = 0, .b = 0, .a = 0},
        .load_op = internal::LoadOp_LOADOP_CLEAR,
        .store_op = internal::StoreOp_STOREOP_STORE};
    backBufferInfo_.color_target_infos.push_back(colorTargetInfo);
    //TODO actually do framebuffering
    return ImportStatus::SUCCESS;
}


} // namespace novus
