#include "gl/scene.h"

#include "gl/debug.h"
#include "proto/renderer.pb.h"
#include "gl/shape_primitive.h"

#include "renderer/pipeline.h"
#include "renderer/material.h"

#include <fmt/format.h>
#include <string_view>
#include <string>

namespace gpr5300::gl
{

const Texture& GetTexture(TextureId textureId)
{
    auto& textureManager = static_cast<TextureManager&>(GetTextureManager());
    return textureManager.GetTexture(textureId);
}

Scene::ImportStatus Scene::LoadShaders(
    const google::protobuf::RepeatedPtrField<gpr5300::pb::Shader>& shadersPb)
{
    const auto shadersSize = shadersPb.size();
    shaders_.resize(shadersSize);
    for (int i = 0; i < shadersSize; i++)
    {
        auto& shaderPb = shadersPb.at(i);
        shaders_[i].LoadShader(shaderPb);
    }
}

Scene::ImportStatus Scene::LoadPipelines(
    const google::protobuf::RepeatedPtrField<pb::Pipeline>& pipelines)
{
    const auto pipelinesSize = pipelines.size();
    pipelines_.resize(pipelinesSize);
    for (int i = 0; i < pipelinesSize; i++)
    {
        auto& pipelinePb = pipelines.at(i);
        pipelines_[i].SetPipelineName(pipelinePb.name());
        switch (pipelinePb.type())
        {
        case pb::Pipeline_Type_RASTERIZE:
            pipelines_[i].LoadRasterizePipeline(shaders_[pipelinePb.vertex_shader_index()],
                shaders_[pipelinePb.fragment_shader_index()]);
            break;
        case pb::Pipeline_Type_COMPUTE:
            pipelines_[i].LoadComputePipeline(shaders_[pipelinePb.compute_shader_index()]);
            break;
        default:
            break;
        }
    }
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<pb::Texture>& textures)
{
    const auto texturesSize = textures.size();
    textures_.resize(texturesSize);
    auto& textureManager = GetTextureManager();
    for (int i = 0; i < texturesSize; i++)
    {
        textures_[i] = { textureManager.LoadTexture(scene_.textures(i)) };
    }
}
Scene::ImportStatus Scene::LoadMaterials(const PbRepeatField<gpr5300::pb::Material>& materials)
{
    const auto materialsSize = scene_.materials_size();
    materials_.resize(materialsSize);
    for (int i = 0; i < materialsSize; i++)
    {
        const auto& materialInfo = scene_.materials(i);
        auto& material = materials_[i];
        material.name = materialInfo.name();
        material.pipelineIndex = materialInfo.pipeline_index();
        const auto materialTexturesCount = materialInfo.textures_size();
        material.textures.resize(materialTexturesCount);
        for (int j = 0; j < materialTexturesCount; j++)
        {
            const auto& materialTextureInfo = materialInfo.textures(j);
            auto& materialTexture = material.textures[j];
            materialTexture.uniformSamplerName = materialTextureInfo.sampler_name();
            if (materialTextureInfo.texture_index() != -1)
            {
                materialTexture.textureId = textures_[materialTextureInfo.texture_index()].textureId;
            }
            else
            {
                materialTexture.attachmentName = materialTextureInfo.attachment_name();
                materialTexture.framebufferName = materialTextureInfo.framebuffer_name();
            }
        }
    }
}

    Scene::ImportStatus Scene::LoadModels(const PbRepeatField<std::string>& models)
    {
        const auto modelsSize = models.size();
        models_.resize(modelsSize);
        for (int i = 0; i < modelsSize; i++)
        {
            models_[i].LoadModel(models.Get(i));
        }
    }

    Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<pb::Mesh>& meshes)
    {
        const auto meshesSize = meshes.size();
        meshes_.resize(meshesSize);
        for (int i = 0; i < meshesSize; i++)
        {
            const auto& mesh = meshes.Get(i);
            switch (mesh.primitve_type())
            {
                case pb::Mesh_PrimitveType_QUAD:
                {
                    const glm::vec3 scale = mesh.has_scale() ? glm::vec3{ mesh.scale().x(), mesh.scale().y(), mesh.scale().z() } : glm::vec3(1.0f);
                    const glm::vec3 offset{ mesh.offset().x(), mesh.offset().y(), mesh.offset().z() };
                    meshes_[i] = GenerateQuad(scale, offset);
                    break;
                }
                case pb::Mesh_PrimitveType_CUBE:
                {
                    const glm::vec3 scale = mesh.has_scale() ? glm::vec3{ mesh.scale().x(), mesh.scale().y(), mesh.scale().z() } : glm::vec3(1.0f);
                    const glm::vec3 offset{ mesh.offset().x(), mesh.offset().y(), mesh.offset().z() };

                    meshes_[i] = GenerateCube(scale, offset);
                    break;
                }
                case pb::Mesh_PrimitveType_SPHERE:
                    break;
                case pb::Mesh_PrimitveType_NONE:
                    meshes_[i] = GenerateEmpty();
                    break;
                case pb::Mesh_PrimitveType_MODEL:
                    meshes_[i] = models_[mesh.model_index()].GenerateMesh(mesh.mesh_name());
                    break;
                default:
                    break;
            }
        }
    }

    Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<pb::FrameBuffer>& framebuffers)
    {
        const auto framebufferSize = framebuffers.size();
        framebuffers_.resize(framebufferSize);
        for (int i = 0; i < framebufferSize; i++)
        {
            framebuffers_[i].Load(framebuffers.Get(i));
        }
    }


    void SceneMaterial::Bind() const
    {
        pipeline_->Bind();
        auto* glMaterial = static_cast<gl::Material*>(material_);
        auto* glPipeline = static_cast<gl::Pipeline*>(pipeline_);
        for (std::size_t textureIndex = 0; textureIndex < glMaterial->textures.size(); textureIndex++)
        {
            glPipeline->SetTexture(
                glMaterial->textures[textureIndex].uniformSamplerName,
                GetTexture(glMaterial->textures[textureIndex].textureId),
                    textureIndex);
        }
    }

    void Scene::UnloadScene()
    {
        for (auto& shader : shaders_)
        {
            shader.Destroy();
        }
        for (auto& pipeline : pipelines_)
        {
            pipeline.Destroy();
        }
    //TODO delete Texture
        for (auto& framebuffer : framebuffers_)
        {
            framebuffer.Destroy();
        }
        for (auto& mesh : meshes_)
        {
            glDeleteVertexArrays(1, &mesh.vao);
        }
    }

    void Scene::Update(float dt)
    {
        glCheckError();
        const auto subPassSize = scene_.render_pass().sub_passes_size();
        for (int i = 0; i < subPassSize; i++)
        {
            const auto& subPass = scene_.render_pass().sub_passes(i);
            const auto framebufferIndex = subPass.framebuffer_index();
            if (framebufferIndex != -1 && framebuffers_.size() > framebufferIndex)
            {
                framebuffers_[framebufferIndex].Bind();
            }
            else
            {
                Framebuffer::Unbind();
            }
            glClearColor(subPass.clear_color().r(),
                         subPass.clear_color().g(),
                         subPass.clear_color().b(),
                         subPass.clear_color().a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            for (auto* pySystem : pySystems_)
            {
                if (pySystem != nullptr)
                {
                    pySystem->Draw(i);
                }
            }

            const auto commandSize = subPass.commands_size();
            for (int j = 0; j < commandSize; j++)
            {
                const auto& command = subPass.commands(j);
                if (!command.automatic_draw())
                    continue;
                Draw(command);
            }
        }

        glCheckError();
    }

    void Scene::Draw(const pb::DrawCommand& command)
    {
        const auto& material = materials_[command.material_index()];
        auto& pipeline = pipelines_[material.pipelineIndex];
        auto& pipelineInfo = scene_.pipelines(material.pipelineIndex);

        if (pipelineInfo.depth_test_enable())
        {
            glEnable(GL_DEPTH_TEST);
            switch (pipelineInfo.depth_compare_op())
            {
                case pb::Pipeline_DepthCompareOp_LESS:
                    glDepthFunc(GL_LESS);
                    break;
                case pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL:
                    glDepthFunc(GL_LEQUAL);
                    break;
                case pb::Pipeline_DepthCompareOp_EQUAL:
                    glDepthFunc(GL_EQUAL);
                    break;
                case pb::Pipeline_DepthCompareOp_GREATER:
                    glDepthFunc(GL_GREATER);
                    break;
                case pb::Pipeline_DepthCompareOp_NOT_EQUAL:
                    glDepthFunc(GL_NOTEQUAL);
                    break;
                case pb::Pipeline_DepthCompareOp_GREATER_OR_EQUAL:
                    glDepthFunc(GL_GEQUAL);
                    break;
                case pb::Pipeline_DepthCompareOp_ALWAYS:
                    glDepthFunc(GL_ALWAYS);
                    break;
                case pb::Pipeline_DepthCompareOp_NEVER:
                    glDepthFunc(GL_NEVER);
                    break;
                default:;
            }
            glDepthMask(pipelineInfo.depth_mask());
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        if (pipelineInfo.blend_enable())
        {
            glEnable(GL_BLEND);
            auto translateBlendFunc = [](pb::Pipeline_BlendFunc blendFunc)
            {
                switch (blendFunc)
                {
                    case pb::Pipeline_BlendFunc_BLEND_ZERO: return GL_ZERO;
                    case pb::Pipeline_BlendFunc_ONE: return GL_ONE;
                    case pb::Pipeline_BlendFunc_SRC_COLOR: return GL_SRC_COLOR;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
                    case pb::Pipeline_BlendFunc_DST_COLOR: return GL_DST_COLOR;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
                    case pb::Pipeline_BlendFunc_SRC_ALPHA: return GL_SRC_ALPHA;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
                    case pb::Pipeline_BlendFunc_DST_ALPHA: return GL_DST_ALPHA;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
                    case pb::Pipeline_BlendFunc_CONSTANT_COLOR: return GL_CONSTANT_COLOR;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_COLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
                    case pb::Pipeline_BlendFunc_CONSTANT_ALPHA: return GL_CONSTANT_ALPHA;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_CONSTANT_ALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
                    case pb::Pipeline_BlendFunc_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
                    case pb::Pipeline_BlendFunc_SRC1_COLOR: return GL_SRC1_COLOR;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_COLOR: return GL_ONE_MINUS_SRC1_COLOR;
                    case pb::Pipeline_BlendFunc_SRC1_ALPHA: return GL_SRC1_ALPHA;
                    case pb::Pipeline_BlendFunc_ONE_MINUS_SRC1_ALPHA: return GL_ONE_MINUS_SRC1_ALPHA;
                    default:;
                }
                return GL_ZERO;
            };
            glBlendFunc(translateBlendFunc(pipelineInfo.blending_source_factor()), translateBlendFunc(pipelineInfo.blending_destination_factor()));
        }
        else
        {
            glDisable(GL_BLEND);
        }
        if (pipelineInfo.enable_stencil_test())
        {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(pipelineInfo.stencil_mask());
            auto translateStencilOp = [](pb::Pipeline_StencilOp stencilOp)
            {
                switch (stencilOp)
                {
                    case pb::Pipeline_StencilOp_KEEP: return GL_KEEP;
                    case pb::Pipeline_StencilOp_STENCIL_ZERO: return GL_ZERO;
                    case pb::Pipeline_StencilOp_REPLACE: return GL_REPLACE;
                    case pb::Pipeline_StencilOp_INCR: return GL_INCR;
                    case pb::Pipeline_StencilOp_INCR_WRAP: return GL_INCR_WRAP;
                    case pb::Pipeline_StencilOp_DECR: return GL_DECR;
                    case pb::Pipeline_StencilOp_DECR_WRAP: return GL_DECR_WRAP;
                    case pb::Pipeline_StencilOp_INVERT: return GL_INVERT;
                    default:;
                }
                return GL_KEEP;
            };
            glStencilOp(
                    translateStencilOp(pipelineInfo.stencil_source_fail()),
                    translateStencilOp(pipelineInfo.stencil_depth_fail()),
                    translateStencilOp(pipelineInfo.stencil_depth_pass())
            );
            static constexpr std::array stencilFunc =
                    {
                            GL_NEVER,
                            GL_LESS,
                            GL_LEQUAL,
                            GL_GREATER,
                            GL_GEQUAL,
                            GL_EQUAL,
                            GL_NOTEQUAL,
                            GL_ALWAYS
                    };
            glStencilFunc(stencilFunc[pipelineInfo.stencil_func()],
                          pipelineInfo.stencil_ref(),
                          pipelineInfo.stencil_func_mask());
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }

        if (pipelineInfo.enable_culling())
        {
            glEnable(GL_CULL_FACE);
            switch (pipelineInfo.cull_face())
            {

                case pb::Pipeline_CullFace_BACK:
                    glCullFace(GL_BACK);
                    break;
                case pb::Pipeline_CullFace_FRONT:
                    glCullFace(GL_FRONT);
                    break;
                case pb::Pipeline_CullFace_FRONT_AND_BACK:
                    glCullFace(GL_FRONT_AND_BACK);
                    break;
                default:
                    break;
            }
            switch (pipelineInfo.front_face())
            {
                case pb::Pipeline_FrontFace_COUNTER_CLOCKWISE:
                    glFrontFace(GL_CCW);
                    break;
                case pb::Pipeline_FrontFace_CLOCKWISE:
                    glFrontFace(GL_CW);
                    break;
                default:
                    break;
            }
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        pipeline.Bind();
        for (std::size_t textureIndex = 0; textureIndex < material.textures.size(); textureIndex++)
        {
            const auto& materialTexture = material.textures[textureIndex];
            if (materialTexture.textureId != INVALID_TEXTURE_ID)
            {
                pipeline.SetTexture(materialTexture.uniformSamplerName, GetTexture(material.textures[textureIndex].textureId), textureIndex);
            }
            else
            {

                const auto& framebufferName = materialTexture.framebufferName;
                if (framebufferName.empty())
                {
                    LogWarning("Empty Framebuffer Name for Material Attachment");
                    continue;
                }
                auto it = std::ranges::find_if(framebuffers_, [&framebufferName](const auto& framebuffer)
                {
                    return framebuffer.GetName() == framebufferName;
                });
                if (it == framebuffers_.end())
                {
                    LogWarning(fmt::format("Could not find framebuffer: {}", framebufferName));
                    continue;
                }
                const auto textureName = it->GetTextureName(materialTexture.attachmentName);
                if (textureName == 0)
                {
                    LogWarning(fmt::format("Could not find attachment: {} in framebuffer: {}", materialTexture.attachmentName, framebufferName));

                    continue;
                }
                pipeline.SetTexture(materialTexture.uniformSamplerName, textureName, textureIndex);


            }

        }


        GLenum mode = 0;
        switch (command.mode())
        {
            case pb::DrawCommand_Mode_TRIANGLES:
                mode = GL_TRIANGLES;
                break;
            default:
                break;
        }

        const auto meshIndex = command.mesh_index();
        if (meshIndex >= 0)
        {
            glBindVertexArray(meshes_[meshIndex].vao);
        }

        if (command.draw_elements())
        {
            glDrawElements(mode, command.count(), GL_UNSIGNED_INT, nullptr);
            glCheckError();
        }
        else
        {
            glDrawArrays(mode, 0, command.count());
            glCheckError();
        }
    }

    std::unique_ptr<gpr5300::SceneMaterial> Scene::GetMaterial(int materialIndex)
    {
        return  std::make_unique<gpr5300::gl::SceneMaterial>(
                &pipelines_[materials_[materialIndex].pipelineIndex],
                &materials_[materialIndex]);
    }

    void Scene::OnEvent(SDL_Event& event)
    {
        gpr5300::Scene::OnEvent(event);
        switch (event.type)
        {
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        glm::uvec2 newWindowSize;
                        newWindowSize.x = event.window.data1;
                        newWindowSize.y = event.window.data2;
                        for (auto& framebuffer : framebuffers_)
                        {
                            framebuffer.Resize(newWindowSize);
                        }
                        break;
                    }
                }
            }
        }
    }
}



