
#ifndef NEKO2_SAMPLE_BROWSER_H
#define NEKO2_SAMPLE_BROWSER_H

#include "engine/engine.h"
#include "novus/scene.h"

namespace sample
{

struct Sample
{
    std::string sceneName;
    novus::renderer::SceneT sceneInfo;
    novus::Scene scene;
};

class SampleBrowser: public core::OnEventInterface, public core::OnGuiInterface, public core::System
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event& event) override;
    void OnGui() override;
private:
    std::vector<Sample> samples_;
    int currentIndex_ = -1;
    core::SceneManager sceneManager_;
};

novus::renderer::SceneT Scene01();
novus::renderer::SceneT Scene02();
novus::renderer::SceneT Scene03();
}


#endif // NEKO2_SAMPLE_BROWSER_H
