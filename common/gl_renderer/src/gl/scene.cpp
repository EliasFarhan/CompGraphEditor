#include "gl/scene.h"

#include "gl/utils.h"

#include "gl/debug.h"
#include "proto/renderer.pb.h"

#include "renderer/draw_command.h"

#include "renderer/pipeline.h"
#include "renderer/material.h"

#include <fmt/format.h>
#include <string_view>
#include <string>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#endif

namespace gl
{

const Texture& GetTexture(core::TextureId textureId)
{
    auto& textureManager = static_cast<TextureManager&>(core::GetTextureManager());
    return textureManager.GetTexture(textureId);
}

core::DrawCommand& Scene::GetDrawCommand(int subPassIndex, int drawCommandIndex)
{
    int resultIndex = 0;
    for(int i = 0; i < this->scene_.render_pass().sub_passes_size(); i++)
    {
        if(i < subPassIndex)
        {
            resultIndex += scene_.render_pass().sub_passes(i).commands_size();
        }
        else if(i == subPassIndex)
        {
            break;
            
        }
    }
    return drawCommands_[resultIndex + drawCommandIndex];
}

Scene::ImportStatus Scene::LoadShaders(
    const google::protobuf::RepeatedPtrField<core::pb::Shader>& shadersPb)
{
    const auto shadersSize = shadersPb.size();
    shaders_.resize(shadersSize);
    for (int i = 0; i < shadersSize; i++)
    {
        auto& shaderPb = shadersPb.at(i);
        shaders_[i].LoadShader(shaderPb);
    }

    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadPipelines(
    const google::protobuf::RepeatedPtrField<core::pb::Pipeline>& pipelines)
{
    const auto pipelinesSize = pipelines.size();
    pipelines_.resize(pipelinesSize);
    for (int i = 0; i < pipelinesSize; i++)
    {
        auto& pipelinePb = pipelines.at(i);
        pipelines_[i].SetPipelineName(pipelinePb.name());
        switch (pipelinePb.type())
        {
        case core::pb::Pipeline_Type_RASTERIZE:
            pipelines_[i].LoadRasterizePipeline(shaders_[pipelinePb.vertex_shader_index()],
                shaders_[pipelinePb.fragment_shader_index()]);
            break;
        case core::pb::Pipeline_Type_COMPUTE:
            pipelines_[i].LoadComputePipeline(shaders_[pipelinePb.compute_shader_index()]);
            break;
        default:
            return ImportStatus::FAILURE;
        }
    }

    return ImportStatus::SUCCESS;
}

Scene::ImportStatus Scene::LoadTextures(const PbRepeatField<core::pb::Texture>& textures)
{
    const auto texturesSize = textures.size();
    textures_.resize(texturesSize);
    auto& textureManager = core::GetTextureManager();
    for (int i = 0; i < texturesSize; i++)
    {
        textures_[i] = { textureManager.LoadTexture(scene_.textures(i)) };
    }
    return ImportStatus::SUCCESS;
}
Scene::ImportStatus Scene::LoadMaterials(const PbRepeatField<core::pb::Material>& materials)
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
                materialTexture.textureId = core::INVALID_TEXTURE_ID;
                materialTexture.attachmentName = materialTextureInfo.attachment_name();
                materialTexture.framebufferName = materialTextureInfo.framebuffer_name();
            }
        }
    }

    return ImportStatus::SUCCESS;
}

    Scene::ImportStatus Scene::LoadModels(const PbRepeatField<std::string>& models)
    {
        auto& modelManager = core::GetModelManager();
        const auto modelsSize = models.size();
        for (int i = 0; i < modelsSize; i++)
        {
            modelIndices_.push_back(modelManager.ImportModel(models.Get(i)));
        }

        return ImportStatus::SUCCESS;
    }

    Scene::ImportStatus Scene::LoadMeshes(const PbRepeatField<core::pb::Mesh>& meshes)
    {

        auto& modelManager = core::GetModelManager();
        const auto meshesSize = meshes.size();
        for (int i = 0; i < meshesSize; i++)
        {
            const auto& meshInfo = meshes.Get(i);
            switch (meshInfo.primitve_type())
            {
                case core::pb::Mesh_PrimitveType_QUAD:
                {
                    const glm::vec3 scale = meshInfo.has_scale() ? glm::vec3{ meshInfo.scale().x(), meshInfo.scale().y(), meshInfo.scale().z() } : glm::vec3(1.0f);
                    const glm::vec3 offset{ meshInfo.offset().x(), meshInfo.offset().y(), meshInfo.offset().z() };
                    const auto mesh = core::GenerateQuad(scale, offset);
                    vertexBuffers_.emplace_back();
                    vertexBuffers_.back().CreateFromMesh(mesh);
                    break;
                }
                case core::pb::Mesh_PrimitveType_CUBE:
                {
                    const glm::vec3 scale = meshInfo.has_scale() ? glm::vec3{ meshInfo.scale().x(), meshInfo.scale().y(), meshInfo.scale().z() } : glm::vec3(1.0f);
                    const glm::vec3 offset{ meshInfo.offset().x(), meshInfo.offset().y(), meshInfo.offset().z() };

                    const auto mesh = core::GenerateCube(scale, offset);
                    vertexBuffers_.emplace_back();
                    vertexBuffers_.back().CreateFromMesh(mesh);
                    break;
                }
                case core::pb::Mesh_PrimitveType_SPHERE:
                    break;
                case core::pb::Mesh_PrimitveType_NONE:
                {
                    vertexBuffers_.emplace_back();
                    vertexBuffers_.back().CreateFromMesh({});
                    break;
                }
                case core::pb::Mesh_PrimitveType_MODEL:
                {
                    const auto& mesh = modelManager.GetModel(modelIndices_[meshInfo.model_index()]).GetMesh(meshInfo.mesh_name());
                    vertexBuffers_.emplace_back();
                    vertexBuffers_.back().CreateFromMesh(mesh);
                    break;
                }
                default:
                    break;
            }
        }

        glCreateVertexArrays(1, &emptyMeshVao_);

        return ImportStatus::SUCCESS;
    }

    Scene::ImportStatus Scene::LoadFramebuffers(const PbRepeatField<core::pb::FrameBuffer>& framebuffers)
    {
        const auto framebufferSize = framebuffers.size();
        framebuffers_.resize(framebufferSize);
        for (int i = 0; i < framebufferSize; i++)
        {
            framebuffers_[i].Load(framebuffers.Get(i));
        }

        return ImportStatus::SUCCESS;
    }

    Scene::ImportStatus Scene::LoadRenderPass(const core::pb::RenderPass& renderPass)
    {
        return ImportStatus::SUCCESS;
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
        auto& textureManager = core::GetTextureManager();
        textureManager.Clear();
        for (auto& framebuffer : framebuffers_)
        {
            framebuffer.Destroy();
        }
        for (auto& vertexBuffer : vertexBuffers_)
        {
            vertexBuffer.Destroy();
        }
        modelIndices_.clear();
        auto& modelManager = core::GetModelManager();
        modelManager.Clear();
        glDeleteVertexArrays(1, &emptyMeshVao_);
    }

    void Scene::Update(float dt)
    {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        glCheckError();
        const auto subPassSize = scene_.render_pass().sub_passes_size();
        for (int i = 0; i < subPassSize; i++)
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(subpass, "Draw Subpass", true);
#endif
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
#ifdef TRACY_ENABLE
            TracyCZoneN(glClearZone, "glClear", true);
#endif

            glClearColor(subPass.clear_color().r(),
                subPass.clear_color().g(),
                subPass.clear_color().b(),
                subPass.clear_color().a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#ifdef TRACY_ENABLE
            TracyCZoneEnd(glClearZone);
            TracyCZoneN(pySystemsDrawZone, "PySystem Draw", true);
#endif

            const auto commandSize = subPass.commands_size();
            for (int j = 0; j < commandSize; j++)
            {
                const auto& command = subPass.commands(j);
                for (auto* pySystem : pySystems_)
                {
                    if (pySystem != nullptr)
                    {
                        pySystem->Draw(&GetDrawCommand(i, j));
                    }
                }
            }
#ifdef TRACY_ENABLE
            TracyCZoneEnd(pySystemsDrawZone);
#endif
            for (int j = 0; j < commandSize; j++)
            {
                const auto& command = subPass.commands(j);
                if (!command.automatic_draw())
                    continue;
                Draw(GetDrawCommand(i, j));
            }
        }

        glCheckError();
    }



    void Scene::Draw(core::DrawCommand& command)
    {
#ifdef TRACY_ENABLE
        ZoneScoped;
#endif
        auto& glCommand = reinterpret_cast<DrawCommand&>(command);
        const auto& commandInfo = command.GetInfo();
        const auto& material = materials_[command.GetMaterialIndex()];
        auto& pipeline = pipelines_[material.pipelineIndex];
        auto& pipelineInfo = scene_.pipelines(material.pipelineIndex);

        if (pipelineInfo.depth_test_enable())
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(ConvertDepthCompareOpToGL(pipelineInfo.depth_compare_op()));
            glDepthMask(pipelineInfo.depth_mask());
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        if (pipelineInfo.blend_enable())
        {
            glEnable(GL_BLEND);
            glBlendFunc(ConvertBlendFuncToGL(pipelineInfo.blending_source_factor()), ConvertBlendFuncToGL(pipelineInfo.blending_destination_factor()));
        }
        else
        {
            glDisable(GL_BLEND);
        }
        if (pipelineInfo.enable_stencil_test())
        {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(pipelineInfo.stencil_mask());
            glStencilOp(
                ConvertStencilOpToGL(pipelineInfo.stencil_source_fail()),
                ConvertStencilOpToGL(pipelineInfo.stencil_depth_fail()),
                ConvertStencilOpToGL(pipelineInfo.stencil_depth_pass())
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
            glCullFace(ConvertCullFaceToGL(pipelineInfo.cull_face()));
            glFrontFace(ConvertFrontFaceToGL(pipelineInfo.front_face()));
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        pipeline.Bind();
        for (std::size_t textureIndex = 0; textureIndex < material.textures.size(); textureIndex++)
        {
            const auto& materialTexture = material.textures[textureIndex];
            if (materialTexture.textureId != core::INVALID_TEXTURE_ID)
            {
                glCommand.SetTexture(materialTexture.uniformSamplerName, GetTexture(material.textures[textureIndex].textureId), textureIndex);
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
                glCommand.SetTexture(materialTexture.uniformSamplerName, textureName, textureIndex);


            }

        }


        GLenum mode = 0;
        switch (commandInfo.mode())
        {
            case core::pb::DrawCommand_Mode_TRIANGLES:
                mode = GL_TRIANGLES;
                break;
            default:
                break;
        }

        const auto meshIndex = command.GetMeshIndex();
        if (meshIndex >= 0)
        {
            vertexBuffers_[meshIndex].Bind();
            glCheckError();
        }
        else
        {
            glBindVertexArray(emptyMeshVao_);
            glCheckError();
        }

        if (commandInfo.draw_elements())
        {
            glDrawElements(mode, commandInfo.count(), GL_UNSIGNED_INT, nullptr);
            glCheckError();
        }
        else
        {
            glDrawArrays(mode, 0, commandInfo.count());
            glCheckError();
        }
    }

    core::SceneMaterial Scene::GetMaterial(int materialIndex)
    {
        return  { &pipelines_[materials_[materialIndex].pipelineIndex],
                &materials_[materialIndex] };
    }

    void Scene::OnEvent(SDL_Event& event)
    {
        core::Scene::OnEvent(event);
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

Scene::ImportStatus Scene::LoadDrawCommands(const core::pb::RenderPass &renderPass)
{
    for(int i = 0; i < renderPass.sub_passes_size(); i++)
    {
        const auto& subpassInfo = renderPass.sub_passes(i);

        for(int j = 0; j < subpassInfo.commands_size(); j++)
        {
            const auto& commandInfo = subpassInfo.commands(j);
            drawCommands_.emplace_back(commandInfo, i);
        }
    }
    return ImportStatus::SUCCESS;
}
}



