#pragma once
#include "engine/scene.h"
#include "vk/pipeline.h"
#include "vk/framebuffer.h"
#include "vk/mesh.h"
#include "vk/common.h"

#include <vulkan/vulkan.h>

namespace gpr5300::vk
{
class Scene : public gpr5300::Scene
{
public:
    void UnloadScene() override;
    void Update(float dt) override;
    void Draw(const pb::DrawCommand& drawCommand) override;
    Framebuffer& GetFramebuffer(int framebufferIndex) override;
    std::unique_ptr<SceneMaterial> GetMaterial(int materialIndex) override;
    Pipeline& GetPipeline(int index) override;
    Mesh& GetMesh(int index) override;
    VkRenderPass GetCurrentRenderPass() const;
protected:
    ImportStatus LoadShaders(const PbRepeatField<pb::Shader>& shadersPb) override;
    ImportStatus LoadPipelines(const PbRepeatField<pb::Pipeline>& pipelines) override;
    ImportStatus LoadTextures(const PbRepeatField<pb::Texture>& textures) override;
    ImportStatus LoadMaterials(const PbRepeatField<pb::Material>& materials) override;
    ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;
    ImportStatus LoadMeshes(const PbRepeatField<pb::Mesh>& meshes) override;
    ImportStatus LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers) override;
    ImportStatus LoadRenderPass(const pb::RenderPass& renderPass) override;
private:
    std::vector<Pipeline> pipelines_;
    std::vector<Framebuffer> framebuffers_;
    std::vector<Mesh> meshes_;
    std::vector<Shader> shaders_;
    VkRenderPass renderPass_{};
};

VkRenderPass GetCurrentRenderPass();
}
