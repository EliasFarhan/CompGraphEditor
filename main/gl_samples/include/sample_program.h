#pragma once
#include <array>

#include "engine/system.h"
#include "renderer/pipeline.h"
#include "gl/scene.h"
#include "engine/engine.h"

namespace gpr5300
{
struct Sample
{
    std::string sceneName;
    core::pb::Scene sceneInfo;
    gl::Scene scene;
};
class SampleBrowserProgram : public core::System, public core::ImguiDrawInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void DrawImGui() override;

private:
    core::SceneManager sceneManager_;
    std::vector<Sample> samples_;
    int currentIndex_ = 0;
};   
} // namespace gpr5300
