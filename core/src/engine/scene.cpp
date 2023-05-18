#include "engine/scene.h"


#include "renderer/texture.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/pipeline.h"
#include "renderer/framebuffer.h"
#include "renderer/model.h"

#include <SDL.h>
#include <fmt/format.h>


namespace core
{

static SceneManager* sceneManagerInstance = nullptr;

void Scene::LoadScene()
{
    const auto& textures = scene_.textures();
    if (LoadTextures(textures) != ImportStatus::SUCCESS)
    {
        LogError("Could not import textures");
    }
    const auto& models = scene_.model_paths();
    if (LoadModels(models) != ImportStatus::SUCCESS)
    {
        LogError("Could not import models");
    }

    const auto& meshes = scene_.meshes();
    if (LoadMeshes(meshes) != ImportStatus::SUCCESS)
    {
        LogError("Could not import meshes");
    }
    const auto& framebuffers = scene_.framebuffers();
    if (LoadFramebuffers(framebuffers) != ImportStatus::SUCCESS)
    {
        LogError("Could not import framebuffers");
    }
    const auto& renderPass = scene_.render_pass();
    if (LoadRenderPass(renderPass) != ImportStatus::SUCCESS)
    {
        LogError("Count not import render pass");
    }
    const auto& shaders = scene_.shaders();
    if (LoadShaders(shaders) != ImportStatus::SUCCESS)
    {
        LogError("Could not import shaders");
    }
    const auto& pipelines = scene_.pipelines();
    const auto& raytracingPipelines = scene_.raytracing_pipelines();
    if(LoadPipelines(pipelines, raytracingPipelines) != ImportStatus::SUCCESS)
    {
        LogError("Could not import pipelines");
    }
    

    const auto& materials = scene_.materials();
    if(LoadMaterials(materials) != ImportStatus::SUCCESS)
    {
        LogError("Could not import materials");
    }

    if(LoadDrawCommands(renderPass) != ImportStatus::SUCCESS)
    {
        LogError("Could not import draw commands");
    }

    const auto systemSize = scene_.systems_size();
    scripts_.clear();
    scripts_.reserve(systemSize);

    auto& scriptLoader = ScriptLoaderLocator::get();

    for(int i = 0; i < systemSize; i++)
    {
        const auto& pySystem = scene_.systems(i);
        scripts_.push_back(scriptLoader.LoadScript(pySystem.path(), pySystem.module(), pySystem.class_()));
    }
}

void Scene::SetScene(const pb::Scene& scene)
{
    scene_ = scene;
}

void Scene::Update(float dt)
{
    for(auto* script : scripts_)
    {
        if(script != nullptr)
        {
            script->Update(dt);
        }
    }
}

int Scene::GetMeshCount() const
{
    return scene_.meshes_size();
}

void Scene::OnEvent(SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
    {
        for (auto* script : scripts_)
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
        for (auto* script : scripts_)
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
        const glm::vec2 mouseMotion(event.motion.xrel, event.motion.yrel);
        for (auto* script : scripts_)
        {
            if (script != nullptr)
            {
                script->OnMouseMotion(mouseMotion);
            }
        }
        break;
    }
    default:
        break;
    }
    
}

SceneSubPass Scene::GetSubpass(int subPassIndex)
{
    return { *this, scene_.render_pass().sub_passes(subPassIndex), subPassIndex };
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

std::string_view Scene::GetMeshName(int index)
{
    return scene_.meshes(index).mesh_name();
}

void SceneManager::Begin()
{

}

void SceneManager::LoadScene(Scene *scene)
{
    auto& scriptLoader = ScriptLoaderLocator::get();
    if (currentScene_ != nullptr)
    {
        currentScene_->UnloadScene();
        scriptLoader.End();
    }
    scriptLoader.Begin();
    currentScene_ = scene;
    scene->LoadScene();
}

void SceneManager::End()
{
    if (currentScene_ != nullptr)
    {
        currentScene_->UnloadScene();
        currentScene_ = nullptr;
        ScriptLoaderLocator::get().End();
    }
}

void SceneManager::OnEvent(SDL_Event& event)
{
    if(currentScene_ == nullptr)
        return;
    currentScene_->OnEvent(event);
}

Scene* GetCurrentScene()
{
    return sceneManagerInstance->GetCurrentScene();
}


void SceneManager::Update(float dt)
{
    if (currentScene_ == nullptr)
        return;
    currentScene_->Update(dt);
}

SceneMaterial::SceneMaterial(Pipeline* pipeline, Material* material) :
    pipeline_(pipeline),
    material_(material)
{
}

Pipeline* SceneMaterial::GetPipeline() const
{
    return pipeline_;
}

std::string_view SceneMaterial::GetName() const
{
    return material_->GetName();
}

SceneManager::SceneManager()
{
    sceneManagerInstance = this;
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

SceneSubPass::SceneSubPass(Scene& scene, const pb::SubPass& subPass, int subPassIndex) : scene_(scene), subPass_(subPass), subPassIndex_(subPassIndex)
{

}
DrawCommand& SceneSubPass::GetDrawCommand(int drawCommandIndex) const
{
    return scene_.GetDrawCommand(subPassIndex_, drawCommandIndex);
}
}
