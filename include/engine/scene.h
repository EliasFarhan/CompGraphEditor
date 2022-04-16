#pragma once

#include <vector>
#include "renderer/pipeline.h"
#include "engine/system.h"

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
