#pragma once

#include "engine/system.h"
#include "engine/engine.h"
#include "engine/scene.h"

namespace gpr5300
{

class SceneEditor : public System, public ImguiDrawInterface
{
public:
    void DrawImGui() override;
    void Begin() override;
    void Update(float dt) override;
    void End() override;
private:
    void DrawMenuBar();
    void DrawSceneContent();
    void DrawCenterView();
    void DrawInspector();

    pb::Scene scene_;
    Inspector inspector_;



};

}