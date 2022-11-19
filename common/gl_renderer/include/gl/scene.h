#pragma once

#include "engine/scene.h"
#include "gl/pipeline.h"
#include "gl/mesh.h"
#include "gl/model.h"
#include "gl/texture.h"
#include "gl/material.h"

#include <vector>

#include "framebuffer.h"


namespace gpr5300::gl
{

class SceneMaterial : public gpr5300::SceneMaterial
{
public:
    using gpr5300::SceneMaterial::SceneMaterial;
    void Bind() const override;
};

class Scene : public gpr5300::Scene
{
public:
    void UnloadScene() override;
    void Update(float dt) override;
    void Draw(const pb::DrawCommand& drawCommand) override;

    std::unique_ptr<gpr5300::SceneMaterial> GetMaterial(int materialIndex) override;

    void OnEvent(SDL_Event& event) override;

protected:
    ImportStatus LoadShaders(const PbRepeatField<pb::Shader>& shadersPb) override;
    ImportStatus LoadPipelines(const PbRepeatField<pb::Pipeline>& pipelines) override;
    ImportStatus LoadTextures(const PbRepeatField<pb::Texture>& textures) override;
    ImportStatus LoadMaterials(const PbRepeatField<pb::Material>& materials) override;
    ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;
    ImportStatus LoadMeshes(const PbRepeatField<pb::Mesh>& meshes) override;
    ImportStatus LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers) override;
private:
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Model> models_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
    std::vector<Framebuffer> framebuffers_;
};
} // namespace gpr5300::gl
