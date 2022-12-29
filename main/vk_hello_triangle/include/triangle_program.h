#pragma once
#include "engine/engine.h"
#include "engine/scene.h"

namespace gpr5300
{

class HelloTriangleProgram : public System, public ImguiDrawInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void DrawImGui() override;
private:
    SceneManager sceneManager_;
    Scene scene_;
};
}
