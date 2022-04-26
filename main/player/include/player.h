#pragma once
#include "engine/engine.h"
#include "engine/scene.h"

namespace gpr5300
{

class Player : public ImguiDrawInterface, public System
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void DrawImGui() override;
private:
    SceneManager sceneManager_;
    std::vector<std::string> scenePaths_;
    bool sceneLoaded_ = false;
    Scene playerScene_;
};

} // namespace gpr5300
