#pragma once

#include "engine/system.h"
#include "gl/scene.h"
#include "engine/engine.h"
#include "proto/renderer.pb.h"

namespace gpr5300
{
struct Sample
{
    std::string sceneName;
    core::pb::Scene sceneInfo;
    gl::Scene scene;
};
class SampleBrowserProgram : public core::System, public core::OnGuiInterface, public core::OnEventInterface
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnGui() override;
    void OnEvent(SDL_Event& event) override;

private:
    core::SceneManager sceneManager_;
    std::vector<Sample> samples_;
    std::size_t currentIndex_ = 0;
};   
} // namespace gpr5300
