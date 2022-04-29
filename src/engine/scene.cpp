#include "engine/scene.h"
#include "GL/glew.h"
#include "renderer/shape_primitive.h"
#include "renderer/debug.h"

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
        material.pipelineIndex = materialInfo.pipeline_index();
        const auto materialTexturesCount = materialInfo.textures_size();
        material.textures.resize(materialTexturesCount);
        for(int j = 0; j < materialTexturesCount; j++)
        {
            const auto& materialTextureInfo = materialInfo.textures(j);
            auto& materialTexture = material.textures[j];
            materialTexture.texture = textures_[materialTextureInfo.texture_index()];
            materialTexture.uniformSamplerName = materialTextureInfo.sampler_name();
        }
    }

    const auto meshesSize = scene_.meshes_size();
    meshes_.resize(meshesSize);
    for(int i = 0; i < meshesSize; i++)
    {
        const auto& mesh = scene_.meshes(i);
        switch(mesh.primitve_type())
        {
        case pb::Mesh_PrimitveType_QUAD:
            meshes_[i] = GenerateQuad();
            break;
        case pb::Mesh_PrimitveType_CUBE:
            break;
        case pb::Mesh_PrimitveType_SPHERE:
            break;
        case pb::Mesh_PrimitveType_NONE:
            meshes_[i] = GenerateEmpty();
            break;
        default:
            break;
        }
    }

    const auto pySystemSize = scene_.py_systems_size();
    for(int i = 0; i < pySystemSize; i++)
    {
        const auto& pySystem = scene_.py_systems(i);
        pySystems_.push_back(pyManager.LoadScript(pySystem.module(), pySystem.class_()));
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
        glClearColor(subPass.clear_color().r(),
                     subPass.clear_color().g(),
                     subPass.clear_color().b(),
                     subPass.clear_color().a());
        glClear(GL_COLOR_BUFFER_BIT);

        for(auto* pySystem: pySystems_)
        {
            pySystem->Draw(i);
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
    SceneMaterial material{&pipelines_[materialIndex], &materials_[materialIndex] };
    return material;
}
void Scene::Draw(const pb::DrawCommand& command)
{
    auto& material = materials_[command.material_index()];
    auto& pipeline = pipelines_[material.pipelineIndex];

    pipeline.Bind();
    for(std::size_t textureIndex = 0; textureIndex < material.textures.size(); textureIndex++)
    {
        pipeline.SetTexture(material.textures[textureIndex].uniformSamplerName, material.textures[textureIndex].texture, textureIndex);
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
int SceneSubPass::GetDrawCommandCount() const
{
    return subPass_.commands_size();
}
SceneSubPass::SceneSubPass(Scene& scene, const pb::SubPass& subPass) : scene_(scene), subPass_(subPass)
{

}
SceneDrawCommand SceneSubPass::GetDrawCommand(int drawCommandIndex) const
{
    return SceneDrawCommand(scene_, subPass_.commands(drawCommandIndex));
}
}