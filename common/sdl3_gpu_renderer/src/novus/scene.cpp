//
// Created by unite on 10.10.2025.
//
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
        std::array colorTargets = {swapchainTexture};

        for (int64_t subpassIndex = 0; subpassIndex < std::ssize(renderpasses_); subpassIndex++)
        {
            auto& subpass = renderpasses_[subpassIndex];
            const auto& subpassInfo = scene_.sub_passes[subpassIndex];
            //TODO define framebuffer to get the needed textures (like in OpenGL?)
            currentRenderPass_ = GenerateSubPass(commandBuffer, subpassInfo, colorTargets, nullptr);
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
    for (size_t i = 0; i < pipelines_.size(); ++i)
    {

        auto& pipelineInfo = pipelineInfos[i];
        Pipeline& pipeline = pipelines_[i];
        pipeline.Load(pipelineInfos[i],
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
    for (size_t i = 0; i < materials.size(); ++i)
    {
        const auto& materialInfo = materials[i];
        const auto pipelineIndex = materialInfo.pipeline_index;
        const auto& pipelineInfo = scene_.pipelines[pipelineIndex];
        const auto vertexIndex = pipelineInfo.vertex_shader_index;
        const auto fragmentIndex = pipelineInfo.fragment_shader_index;

        Material material(&pipelines_[pipelineIndex]);
        //TODO add the specific texture sampler index? same texture path different texture settings
        material.Load(materialInfo,
            scene_.shaders[vertexIndex],
            scene_.shaders[fragmentIndex],
            bufferManager_);
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
        renderpasses_.emplace_back(renderPass[subpassIndex], (int)subpassIndex, pipelines_, materials_);
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


} // namespace novus
