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
}

core::SceneMaterial Scene::GetMaterial(int materialIndex)
{
}

core::Pipeline& Scene::GetPipeline(int index)
{
}

core::DrawCommand& Scene::GetDrawCommand(int subPassIndex, int drawCommandIndex)
{
}

core::BufferManager& Scene::GetBufferManager()
{
}

Scene::ImportStatus Scene::LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb)
{
}

Scene::ImportStatus Scene::LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines,
	const PbRepeatField<core::pb::RaytracingPipeline>& raytracingPipelines)
{
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<core::pb::Texture>& textures)
{
}

Scene::ImportStatus Scene::LoadMaterials(const PbRepeatField<core::pb::Material>& materials)
{
}

Scene::ImportStatus Scene::LoadModels(const PbRepeatField<std::string>& models)
{
}

Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes)
{
}

Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers)
{
}

Scene::ImportStatus Scene::LoadDrawCommands(const core::pb::RenderPass& renderPass)
{
}

Scene::ImportStatus Scene::LoadRenderPass(const core::pb::RenderPass& renderPass)
{
}

Scene::ImportStatus Scene::LoadBuffers(const PbRepeatField<core::pb::Buffer>& buffers)
{
}
}
