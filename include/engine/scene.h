#pragma once

#include "renderer/pipeline.h"
#include "engine/system.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"
#include "py_interface.h"
#include "renderer/material.h"

#include <vector>

namespace gpr5300
{
class Scene
{
public:
    void LoadScene(PyManager& pyManager);
    void UnloadScene();
    void SetScene(const pb::Scene &scene);
    void Update(float dt);

    Pipeline& GetPipeline(int index){ return pipelines_[index]; }
private:
    pb::Scene scene_;
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
    std::vector<System*> pySystems_;
    //std::vector<SubPass> subpasses_;

};

class SceneManager : public System
{
public:
    SceneManager();
    void LoadScene(Scene* scene);
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    Scene* GetCurrentScene() { return currentScene_; }
    static SceneManager* GetInstance(){ return sceneManager_; }
    TextureManager& GetTextureManager(){ return textureManager_; }
private:
    inline static SceneManager* sceneManager_ = nullptr;
    Scene* currentScene_ = nullptr;
    PyManager pyManager_;
    TextureManager textureManager_;
};

} // namespace gpr5300
