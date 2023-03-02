#pragma once

#include "engine/scene.h"
#include "gl/pipeline.h"
#include "gl/mesh.h"
#include "gl/model.h"
#include "gl/texture.h"
#include "gl/material.h"

#include <vector>

#include "framebuffer.h"


namespace gl
{

class SceneMaterial : public core::SceneMaterial
{
public:
    using core::SceneMaterial::SceneMaterial;
    void Bind() const override;
};


struct SceneTexture
{
    core::TextureId textureId = core::INVALID_TEXTURE_ID;
};

class Scene : public core::Scene
{
public:
    void UnloadScene() override;
    void Update(float dt) override;
    void Draw(const core::pb::DrawCommand& drawCommand) override;

    std::unique_ptr<core::SceneMaterial> GetMaterial(int materialIndex) override;

    void OnEvent(SDL_Event& event) override;
    core::Framebuffer& GetFramebuffer(int framebufferIndex) override { return framebuffers_[framebufferIndex]; }
    core::Pipeline& GetPipeline(int index) override { return pipelines_[index]; }
    core::Mesh& GetMesh(int index) override { return meshes_[index]; }
protected:
    ImportStatus LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb) override;
    ImportStatus LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines) override;
    ImportStatus LoadTextures(const PbRepeatField<core::pb::Texture>& textures) override;
    ImportStatus LoadMaterials(const PbRepeatField<core::pb::Material>& materials) override;
    ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;
    ImportStatus LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes) override;
    ImportStatus LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers) override;
    

private:
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Model> models_;
    std::vector<SceneTexture> textures_;
    std::vector<Material> materials_;
    std::vector<Framebuffer> framebuffers_;
};
} // namespace gl
