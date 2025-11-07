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

    SDL_GPUTexture* swapchainTexture = GetSwapchainTexture();
    if (swapchainTexture != nullptr)
    {
        std::array colorTargets = {swapchainTexture};

        for (const auto& subpass: scene_.render_pass->sub_passes)
        {
            //TODO define framebuffer to get the needed textures (like in OpenGL?)
            SDL_GPURenderPass* renderPass = GenerateSubPass(commandBuffer, subpass, colorTargets, nullptr);
            for (const auto& command: subpass.commands)
            {
                const auto& material = materials_[command.material_index];
                auto& pipeline = pipelines_[material.GetPipelineIndex()];
                pipeline.Bind(renderPass);

                if (command.mesh_index != -1)
                {
                    auto& vertexInputBuffer = vertexInputBuffers_[command.mesh_index];
                    vertexInputBuffer.Bind(renderPass);
                }


                if (command.draw_elements)
                {
                    SDL_DrawGPUIndexedPrimitives(renderPass, command.count, 1, 0, 0, 0);
                }
                else
                {
                    SDL_DrawGPUPrimitives(renderPass, command.count, 1, 0, 0);
                }
            }
            SDL_EndGPURenderPass(renderPass);
        }


    }
}

void Scene::Draw(core::DrawCommand& drawCommand, int instance)
{
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
    return commands_.at(drawCommandIndex);
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
Scene::ImportStatus Scene::LoadPipelines(std::span<const renderer::GraphicsPipelineT> pipelines) {
    pipelines_.resize(pipelines.size());
    for (size_t i = 0; i < pipelines_.size(); ++i)
    {
        auto& pipeline = pipelines[i];
        pipelines_[i].Load(pipelines[i], shaders_[pipeline.vertex_shader_index], shaders_[pipeline.fragment_shader_index]);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadMaterials(std::span<const renderer::MaterialT> materials)
{
    materials_.resize(materials.size());
    for (size_t i = 0; i < materials.size(); ++i)
    {
        //TODO add the specific texture sampler index? same texture path different texture settings
        materials_[i].Load(materials[i]);
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
Scene::ImportStatus Scene::LoadDrawCommands(const renderer::RenderpassT* renderPass)
{
    if (renderPass == nullptr)
    {
        throw std::runtime_error("renderPass is nullptr");
    }
    for (int subpassIndex = 0; subpassIndex < renderPass->sub_passes.size(); ++subpassIndex)
    {
        const auto& subpass = renderPass->sub_passes[subpassIndex];
        for (const auto& command: subpass.commands)
        {
            commands_.push_back({command, subpassIndex});
        }
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadRenderPass(const renderer::RenderpassT* renderPass)
{
    return ImportStatus::SUCCESS;
}


}
