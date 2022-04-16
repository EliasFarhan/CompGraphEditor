#pragma once

#include "renderer/pipeline.h"
#include "engine/system.h"
#include "renderer/mesh.h"
#include "renderer/texture.h"

#include <vector>

namespace gpr5300
{
class Scene
{
public:
    void LoadScene();
    void UnloadScene();
    void SetScene(const pb::Scene &scene);
    void Update(float dt);
private:
    pb::Scene scene_;
    std::vector<Shader> shaders_;
    std::vector<Pipeline> pipelines_;
    std::vector<Mesh> meshes_;
    std::vector<Texture> textures_;
    //std::vector<SubPass> subpasses_;

};

class SceneManager : public System
{
public:
    void LoadScene(Scene* scene);
    void Begin() override;
    void Update(float dt) override;
    void End() override;
private:
    Scene* currentScene_ = nullptr;
};

} // namespace gpr5300
