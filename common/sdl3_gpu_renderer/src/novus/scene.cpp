//
// Created by unite on 10.10.2025.
//
#include "novus/scene.h"

namespace novus
{
void Scene::UnloadScene()
{
}

void Scene::Update(float dt)
{
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
    auto& materialInfo = scene_.materials.at(materialIndex);
    return {&pipelines_[materialInfo.pipeline_index], nullptr};
}

core::Pipeline& Scene::GetPipeline(int index)
{
    return pipelines_[index];
}

core::DrawCommand& Scene::GetDrawCommand(int subPassIndex, int drawCommandIndex)
{
    return commands_.at(drawCommandIndex);
}

Scene::ImportStatus Scene::LoadShaders(std::span<const novus::renderer::ShaderT> shadersPb)
{
    shaders_.resize(shadersPb.size());
    for (size_t i = 0; i < shadersPb.size(); ++i)
    {
        shaders_[i].LoadShader(shadersPb[i]);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadPipelines(std::span<const novus::renderer::GraphicsPipelineT> pipelines) {
    pipelines_.resize(pipelines.size());
    for (size_t i = 0; i < pipelines_.size(); ++i)
    {
        auto& pipeline = pipelines[i];
        pipelines_[i].Load(pipelines[i], shaders_[pipeline.vertex_shader_index], shaders_[pipeline.fragment_shader_index]);
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadMaterials(std::span<const novus::renderer::MaterialT> materials)
{
    return ImportStatus::FAILURE;
}
Scene::ImportStatus Scene::LoadMeshes(std::span<const novus::renderer::MeshT> meshes)
{
    return ImportStatus::FAILURE;
}
Scene::ImportStatus Scene::LoadDrawCommands(const novus::renderer::RenderpassT* renderPass)
{
    return ImportStatus::FAILURE;
}
Scene::ImportStatus Scene::LoadRenderPass(const novus::renderer::RenderpassT* renderPass)
{
    return ImportStatus::FAILURE;
}


}
