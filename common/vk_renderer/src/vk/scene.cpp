#include "vk/scene.h"
#include "vk/utils.h"
#include "vk/window.h"
#include "vk/pipeline.h"

namespace gpr5300::vk
{
void Scene::UnloadScene()
{
}

void Scene::Update(float dt)
{
}

void Scene::Draw(const pb::DrawCommand& drawCommand)
{
}

Framebuffer& Scene::GetFramebuffer(int framebufferIndex)
{
    return framebuffers_[framebufferIndex];
}

std::unique_ptr<SceneMaterial> Scene::GetMaterial(int materialIndex)
{
    return nullptr;
}

Pipeline& Scene::GetPipeline(int index)
{
    return pipelines_[index];
}

Mesh& Scene::GetMesh(int index)
{
    return meshes_[index];
}

VkRenderPass Scene::GetCurrentRenderPass() const
{
    return renderPass_;
}

Scene::ImportStatus Scene::LoadShaders(const PbRepeatField<pb::Shader>& shadersPb)
{
    const auto& driver = GetDriver();
    const auto& filesystem = FilesystemLocator::get();
    for (auto& shaderPb : shadersPb)
    {
        const auto shaderFile = filesystem.LoadFile(shaderPb.path());
        auto shaderModule = CreateShaderModule(shaderFile, driver.device);
        if (!shaderModule)
            return ImportStatus::FAILURE;
        Shader shader{.module = shaderModule.value()};
        switch (shaderPb.type())
        {
        case pb::Shader_Type_VERTEX:
            shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case pb::Shader_Type_FRAGMENT:
            shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case pb::Shader_Type_COMPUTE:
            shader.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        default:
            return ImportStatus::FAILURE;
        }
        shaders_.push_back(shader);
        
    }
    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadPipelines(const PbRepeatField<pb::Pipeline>& pipelines)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<pb::Texture>& textures)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadMaterials(const PbRepeatField<pb::Material>& materials)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadModels(const PbRepeatField<std::string>& models)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<pb::Mesh>& meshes)
{
    return ImportStatus::FAILURE;
}

Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers)
{
    return ImportStatus::FAILURE;
}

VkRenderPass GetCurrentRenderPass()
{
    const auto* scene = static_cast<vk::Scene*>(GetCurrentScene());
    return scene->GetCurrentRenderPass();
}
}
