#pragma once

#include "engine/scene.h"
#include "gl/pipeline.h"
#include "gl/buffer.h"
#include "gl/texture.h"
#include "gl/material.h"
#include "gl/command.h"

#include <vector>

#include "framebuffer.h"


namespace gl
{



struct SceneTexture
{
    core::TextureId textureId = core::INVALID_TEXTURE_ID;
};

class Scene : public core::Scene
{
public:
    void UnloadScene() override;
    void Update(float dt) override;
    void Draw(core::DrawCommand& drawCommand, int instance = 1) override;
    void Dispatch(core::ComputeCommand& command, int x, int y, int z) override;


    core::SceneMaterial GetMaterial(int materialIndex) override;

    void OnEvent(SDL_Event& event) override;
    int GetFramebufferIndex(std::string_view framebufferName);
    core::Framebuffer& GetFramebuffer(int framebufferIndex) override { return framebuffers_[framebufferIndex]; }
    core::Pipeline& GetPipeline(int index) override { return pipelines_[index]; }
    VertexInputBuffer& GetVertexBuffer(int index) { return vertexBuffers_[index]; }
    core::DrawCommand& GetDrawCommand(int subPassIndex, int drawCommandIndex) override;
    core::ComputeCommand& GetComputeCommand(int subpassIndex, int computeCommandIndex);

    core::BufferManager& GetBufferManager() override { return bufferManager_; }
    
protected:
    ImportStatus LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb) override;
    ImportStatus LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines, const PbRepeatField<core::pb::RaytracingPipeline>& raytracingPipelines) override;
    ImportStatus LoadTextures(const PbRepeatField<core::pb::Texture>& textures) override;
    ImportStatus LoadMaterials(const PbRepeatField<core::pb::Material>& materials) override;
    ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;
    ImportStatus LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes) override;
    ImportStatus LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers) override;
    ImportStatus LoadRenderPass(const core::pb::RenderPass& renderPass) override;
    ImportStatus LoadDrawCommands(const core::pb::RenderPass &renderPass) override;
    ImportStatus LoadBuffers(const PbRepeatField<core::pb::Buffer>& buffers) override;
private:
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<VertexInputBuffer> vertexBuffers_;
    std::vector<core::ModelIndex> modelIndices_;
    std::vector<SceneTexture> textures_;
    std::vector<Material> materials_;
    std::vector<Framebuffer> framebuffers_;
    std::vector<DrawCommand> drawCommands_;
    std::vector<ComputeCommand> computeCommands_;
    BufferManager bufferManager_;

    GLuint emptyMeshVao_ = 0;
};
} // namespace gl
