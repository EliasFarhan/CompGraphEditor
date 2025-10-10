//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_SCENE_H
#define NEKO2_SCENE_H
#include "engine/scene.h"

namespace novus
{
class Scene: public core::Scene
{
public:
	void UnloadScene() override;

	void Update(float dt) override;

	void Draw(core::DrawCommand& drawCommand, int instance) override;

	void Dispatch(core::ComputeCommand& command, int x, int y, int z) override;

	core::Framebuffer& GetFramebuffer(int framebufferIndex) override;

	core::SceneMaterial GetMaterial(int materialIndex) override;

	core::Pipeline& GetPipeline(int index) override;

	core::DrawCommand& GetDrawCommand(int subPassIndex, int drawCommandIndex) override;

	core::BufferManager& GetBufferManager() override;

protected:
	ImportStatus LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb) override;

	ImportStatus LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines,
		const PbRepeatField<core::pb::RaytracingPipeline>& raytracingPipelines) override;

	ImportStatus LoadTextures(const PbRepeatField<core::pb::Texture>& textures) override;

	ImportStatus LoadMaterials(const PbRepeatField<core::pb::Material>& materials) override;

	ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;

	ImportStatus LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes) override;

	ImportStatus LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers) override;

	ImportStatus LoadDrawCommands(const core::pb::RenderPass& renderPass) override;

	ImportStatus LoadRenderPass(const core::pb::RenderPass& renderPass) override;

	ImportStatus LoadBuffers(const PbRepeatField<core::pb::Buffer>& buffers) override;
};
}
#endif //NEKO2_SCENE_H
