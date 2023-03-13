#pragma once
#include "engine/scene.h"
#include "vk/pipeline.h"
#include "vk/framebuffer.h"
#include "vk/mesh.h"
#include "vk/common.h"

#include <vulkan/vulkan.h>

namespace vk
{
class Scene : public core::Scene
{
public:
    void UnloadScene() override;
    void Update(float dt) override;
    void Draw(const core::pb::DrawCommand& drawCommand) override;
    Framebuffer& GetFramebuffer(int framebufferIndex) override;
    std::unique_ptr<core::SceneMaterial> GetMaterial(int materialIndex) override;
    Pipeline& GetPipeline(int index) override;
    VkRenderPass GetCurrentRenderPass() const;
protected:
    ImportStatus LoadShaders(const PbRepeatField<core::pb::Shader>& shadersPb) override;
    ImportStatus LoadPipelines(const PbRepeatField<core::pb::Pipeline>& pipelines) override;
    ImportStatus LoadTextures(const PbRepeatField<core::pb::Texture>& textures) override;
    ImportStatus LoadMaterials(const PbRepeatField<core::pb::Material>& materials) override;
    ImportStatus LoadModels(const PbRepeatField<std::string>& models) override;
    ImportStatus LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes) override;
    ImportStatus LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers) override;
    ImportStatus LoadRenderPass(const core::pb::RenderPass& renderPass) override;
private:
    std::vector<Pipeline> pipelines_;
    std::vector<Framebuffer> framebuffers_;
    std::vector<Mesh> meshes_;
    std::vector<Shader> shaders_;
    VkRenderPass renderPass_{};
    std::vector<VkFramebuffer> vkFramebuffers_;
};

VkRenderPass GetCurrentRenderPass();
}
