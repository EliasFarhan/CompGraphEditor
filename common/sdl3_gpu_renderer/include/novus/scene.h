//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_SCENE_H
#define NEKO2_SCENE_H
#include "buffer.h"
#include "command.h"
#include "engine/scene.h"
#include "framebuffer.h"
#include "material.h"
#include "pipeline.h"
#include "render_pass.h"

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


protected:
    ImportStatus LoadShaders(std::span<const renderer::ShaderT> shadersPb) override;
    ImportStatus LoadPipelines(std::span<const renderer::GraphicsPipelineT> pipelines) override;
    ImportStatus LoadMaterials(std::span<const renderer::MaterialT> materials) override;
    ImportStatus LoadMeshes(std::span<const renderer::MeshT> meshes) override;
    ImportStatus LoadRenderPass(std::span<const renderer::RenderpassT> renderPass) override;
private:
    std::vector<Renderpass> renderpasses_;
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Framebuffer> framebuffers_;
    std::vector<Material> materials_;
    std::vector<VertexInputBuffer> vertexInputBuffers_;
    SDL_GPURenderPass* currentRenderPass_ = nullptr;
};
}
#endif //NEKO2_SCENE_H
