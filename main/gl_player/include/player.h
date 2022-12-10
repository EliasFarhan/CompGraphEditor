#pragma once
#include "engine/engine.h"
#include "engine/scene.h"
#include "gl/scene.h"

namespace gpr5300
{

class Player : public ImguiDrawInterface, public System, public OnEventInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void DrawImGui() override;
    void OnEvent(SDL_Event& event) override;

private:
    SceneManager sceneManager_;
    std::vector<std::string> scenePaths_;
    bool sceneLoaded_ = false;
    gl::Scene playerScene_;
};

} // namespace gpr5300
