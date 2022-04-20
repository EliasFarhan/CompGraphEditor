#include "engine/scene.h"
#include "GL/glew.h"
#include "renderer/shape_primitive.h"

namespace gpr5300
{

void Scene::LoadScene()
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
        auto &pipelinePb = scene_.pipelines(i);
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
            break;
        default:
            break;
        }
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
    const auto subPassSize = scene_.render_pass().sub_passes_size();
    for (int i = 0; i < subPassSize; i++)
    {
        const auto &subPass = scene_.render_pass().sub_passes(i);
        glClearColor(subPass.clear_color().r(),
                     subPass.clear_color().g(),
                     subPass.clear_color().b(),
                     subPass.clear_color().a());
        glClear(GL_COLOR_BUFFER_BIT);

        const auto commandSize = subPass.commands_size();
        for (int j = 0; j < commandSize; j++)
        {
            const auto &command = subPass.commands(j);
            pipelines_[command.pipeline_index()].Bind();

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
            }
            else
            {
                glDrawArrays(mode, 0, command.count());
            }
        }
    }
}

void SceneManager::Begin()
{

}

void SceneManager::LoadScene(Scene *scene)
{
    if (currentScene_)
    {
        currentScene_->UnloadScene();
    }
    scene->LoadScene();
    currentScene_ = scene;
}

void SceneManager::End()
{
    currentScene_->UnloadScene();
    currentScene_ = nullptr;
}

void SceneManager::Update(float dt)
{
    if (currentScene_ == nullptr)
        return;
    currentScene_->Update(dt);
}
}