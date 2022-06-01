#include "engine/scene.h"
#include "GL/glew.h"
#include "renderer/shape_primitive.h"
#include "renderer/debug.h"
#include <SDL.h>

namespace gpr5300
{

void Scene::LoadScene(PyManager &pyManager)
{

    const auto shadersSize = scene_.shaders_size();
    shaders_.resize(shadersSize);
    for (int i = 0; i < shadersSize; i++)
    {
        auto &shaderPb = scene_.shaders(i);
        shaders_[i].LoadShader(shaderPb);
    }
    const auto pipelinesSize = scene_.pipelines_size();
    pipelines_.resize(pipelinesSize);
    for (int i = 0; i < pipelinesSize; i++)
    {
        auto& pipelinePb = scene_.pipelines(i);
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

    const auto texturesSize = scene_.textures_size();
    textures_.resize(texturesSize);
    auto& textureManager = SceneManager::GetInstance()->GetTextureManager();
    for(int i = 0; i < texturesSize; i++)
    {
        textures_[i] = textureManager.LoadTexture(scene_.textures(i));
    }

    const auto materialsSize = scene_.materials_size();
    materials_.resize(materialsSize);
    for(int i = 0; i < materialsSize; i++)
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
                materialTexture.texture = textures_[materialTextureInfo.texture_index()];
            }
        }
    }
    
    const auto modelsSize = scene_.model_paths_size();
    models_.resize(modelsSize);
    for(int i = 0; i < modelsSize; i++)
    {
        models_[i].LoadModel(scene_.model_paths(i));
    }

    const auto meshesSize = scene_.meshes_size();
    meshes_.resize(meshesSize);
    for(int i = 0; i < meshesSize; i++)
    {
        const auto& mesh = scene_.meshes(i);
        switch(mesh.primitve_type())
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

    const auto framebufferSize = scene_.framebuffers_size();
    framebuffers_.resize(framebufferSize);
    for(int i = 0; i < framebufferSize; i++)
    {
        framebuffers_[i].Load(scene_.framebuffers(i));
    }

    const auto pySystemSize = scene_.py_systems_size();
    for(int i = 0; i < pySystemSize; i++)
    {
        const auto& pySystem = scene_.py_systems(i);
        pySystems_.push_back(pyManager.LoadScript(pySystem.path(), pySystem.module(), pySystem.class_()));
    }
}

void Scene::UnloadScene()
{
    for (auto &shader : shaders_)
    {
        shader.Destroy();
    }
    for (auto &pipeline : pipelines_)
    {
        pipeline.Destroy();
    }

}

void Scene::SetScene(const pb::Scene &scene)
{
    scene_ = scene;
}

void Scene::Update(float dt)
{
    glCheckError();
    const auto subPassSize = scene_.render_pass().sub_passes_size();
    for (int i = 0; i < subPassSize; i++)
    {
        const auto &subPass = scene_.render_pass().sub_passes(i);
        const auto framebufferIndex = subPass.framebuffer_index();
        if(framebufferIndex != -1 && framebuffers_.size() > framebufferIndex)
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
            const auto &command = subPass.commands(j);
            if(!command.automatic_draw())
                continue;
            Draw(command);
        }
    }

    glCheckError();
}

SceneMaterial Scene::GetMaterial(int materialIndex)
{
    const SceneMaterial material{&pipelines_[materials_[materialIndex].pipelineIndex], &materials_[materialIndex] };
    return material;
}

int Scene::GetMeshCount() const
{
    return scene_.meshes_size();
}

void Scene::Draw(const pb::DrawCommand& command)
{
    const auto& material = materials_[command.material_index()];
    auto& pipeline = pipelines_[material.pipelineIndex];
    auto& pipelineInfo = scene_.pipelines(material.pipelineIndex);

    if(pipelineInfo.depth_test_enable())
    {
        glEnable(GL_DEPTH_TEST);
        switch(pipelineInfo.depth_compare_op())
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
        default: ;
        }
        glDepthMask(pipelineInfo.depth_mask());
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    if(pipelineInfo.blend_enable())
    {
        glEnable(GL_BLEND);
        auto translateBlendFunc = [](pb::Pipeline_BlendFunc blendFunc)
        {
            switch(blendFunc)
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
            default: ;
            }
            return GL_ZERO;
        };
        glBlendFunc(translateBlendFunc(pipelineInfo.blending_source_factor()), translateBlendFunc(pipelineInfo.blending_destination_factor()));
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if(pipelineInfo.enable_stencil_test())
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

    if(pipelineInfo.enable_culling())
    {
        glEnable(GL_CULL_FACE);
        switch(pipelineInfo.cull_face())
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
        switch(pipelineInfo.front_face())
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
        if (material.textures[textureIndex].texture.name != 0)
        {
            pipeline.SetTexture(material.textures[textureIndex].uniformSamplerName, material.textures[textureIndex].texture, textureIndex);
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
    if(meshIndex >= 0)
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

void Scene::OnEvent(SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
    {
        for (auto* script : pySystems_)
        {
            if (script != nullptr)
            {
                script->OnKeyDown(event.key.keysym.sym);
            }
        }
        break;
    }
    case SDL_KEYUP:
    {
        for (auto* script : pySystems_)
        {
            if (script != nullptr)
            {
                script->OnKeyUp(event.key.keysym.sym);
            }
        }
        break;
    }
    case SDL_MOUSEMOTION:
    {
        glm::vec2 mouseMotion(event.motion.xrel, event.motion.yrel);
        for (auto* script : pySystems_)
        {
            if (script != nullptr)
            {
                script->OnMouseMotion(mouseMotion);
            }
        }
        break;
    }
    case SDL_WINDOWEVENT:
    {
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
        {
            glm::uvec2 newWindowSize;
            newWindowSize.x = event.window.data1;
            newWindowSize.y = event.window.data2;
            for (auto& framebuffer: framebuffers_)
            {
                framebuffer.Resize(newWindowSize);
            }
            break;
        }
        }
    }
    default:
        break;
    }
    
}

Framebuffer& Scene::GetFramebuffer(int framebufferIndex)
{
    return framebuffers_[framebufferIndex];
}

SceneSubPass Scene::GetSubpass(int subPassIndex)
{
    return SceneSubPass(*this, scene_.render_pass().sub_passes(subPassIndex));
}
int Scene::GetSubpassCount() const
{
    return scene_.render_pass().sub_passes_size();
}
int Scene::GetMaterialCount() const
{
    return scene_.materials_size();
}
int Scene::GetPipelineCount() const
{
    return scene_.pipelines_size();
}

void SceneManager::Begin()
{

}

void SceneManager::LoadScene(Scene *scene)
{
    if (currentScene_ != nullptr)
    {
        currentScene_->UnloadScene();
        pyManager_.End();
    }
    pyManager_.Begin();
    currentScene_ = scene;
    scene->LoadScene(pyManager_);
}

void SceneManager::End()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->UnloadScene();
        currentScene_ = nullptr;
        pyManager_.End();
    }
}

void SceneManager::OnEvent(SDL_Event& event)
{
    if(currentScene_ == nullptr)
        return;
    currentScene_->OnEvent(event);
}

void SceneManager::Update(float dt)
{
    if (currentScene_ == nullptr)
        return;
    pyManager_.Update(dt);
    currentScene_->Update(dt);
}

SceneMaterial::SceneMaterial(Pipeline* pipeline, Material* material) :
    pipeline_(pipeline),
    material_(material)
{
}

void SceneMaterial::Bind() const
{
    pipeline_->Bind();
    for (std::size_t textureIndex = 0; textureIndex < material_->textures.size(); textureIndex++)
    {
        pipeline_->SetTexture(
            material_->textures[textureIndex].uniformSamplerName,
            material_->textures[textureIndex].texture,
            textureIndex);
    }
}

Pipeline* SceneMaterial::GetPipeline() const
{
    return pipeline_;
}

std::string_view SceneMaterial::GetName() const
{
    return material_->name;
}

SceneManager::SceneManager()
{
    sceneManager_ = this;
}
SceneDrawCommand::SceneDrawCommand(Scene& scene, const pb::DrawCommand& drawCommand) :
    scene_(scene), drawCommand_(drawCommand)
{

}
SceneMaterial SceneDrawCommand::GetMaterial() const
{
    return scene_.GetMaterial(drawCommand_.material_index());
}
void SceneDrawCommand::Draw()
{
    scene_.Draw(drawCommand_);
}

std::string_view SceneDrawCommand::GetMeshName() const
{
    return scene_.GetMesh(drawCommand_.mesh_index()).name;
}

std::string_view SceneDrawCommand::GetName() const
{
    return drawCommand_.name();
}

int SceneSubPass::GetDrawCommandCount() const
{
    return subPass_.commands_size();
}

Framebuffer* SceneSubPass::GetFramebuffer()
{
    if (subPass_.framebuffer_index() != -1)
    {
        return &scene_.GetFramebuffer(subPass_.framebuffer_index());
    }
    return nullptr;
}

SceneSubPass::SceneSubPass(Scene& scene, const pb::SubPass& subPass) : scene_(scene), subPass_(subPass)
{

}
SceneDrawCommand SceneSubPass::GetDrawCommand(int drawCommandIndex) const
{
    return { scene_, subPass_.commands(drawCommandIndex) };
}
}