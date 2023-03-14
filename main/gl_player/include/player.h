#pragma once
#include "engine/engine.h"
#include "engine/scene.h"
#include "gl/scene.h"

namespace gpr5300
{

class Player : public core::OnGuiInterface, public core::System, public core::OnEventInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnGui() override;
    void OnEvent(SDL_Event& event) override;

private:
    core::SceneManager sceneManager_;
    std::vector<std::string> scenePaths_;
    bool sceneLoaded_ = false;
    gl::Scene playerScene_;
};

} // namespace gpr5300
