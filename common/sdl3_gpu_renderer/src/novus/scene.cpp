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
}

void Scene::Update(float dt)
{
    auto* commandBuffer = SDL_AcquireGPUCommandBuffer(GetDevice());
    SDL_GPUTexture* swapchainTexture = nullptr;
    Uint32 swapchainWidth = 0, swapchainHeight = 0;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, GetWindow(), &swapchainTexture, &swapchainWidth, &swapchainHeight);
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
                SDL_BindGPUGraphicsPipeline(renderPass, pipeline.get());
                SDL_DrawGPUPrimitives(renderPass, command.count, 1, 0, 0);
            }
            SDL_EndGPURenderPass(renderPass);
        }


    }
    SDL_SubmitGPUCommandBuffer(commandBuffer);
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
    return ImportStatus::FAILURE;
}
Scene::ImportStatus Scene::LoadDrawCommands(const renderer::RenderpassT* renderPass)
{
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
    return ImportStatus::FAILURE;
}


}
