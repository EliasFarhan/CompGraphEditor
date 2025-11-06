#include "sample.h"

#include <generated/renderer_generated.h>
#include <imgui.h>

#include "novus/mesh.h"
#include "novus/scene.h"

namespace sample
{





void SampleBrowser::Begin()
{
    samples_ = {
        {"scene1", Scene01()},
        {"scene2", Scene02()},
        {"scene3", Scene03()},
    };

    for(auto& sample : samples_)
    {
        sample.scene.SetScene(sample.sceneInfo);
    }
    currentIndex_ = 1;
    sceneManager_.LoadScene(&samples_[currentIndex_].scene);
}
void SampleBrowser::Update(float dt)
{
    sceneManager_.Update(dt);
}
void SampleBrowser::End()
{
    sceneManager_.End();
}
void SampleBrowser::OnEvent(SDL_Event& event)
{
    sceneManager_.OnEvent(event);
}
void SampleBrowser::OnGui()
{
    ImGui::Begin("Sample Browser");

    ImGui::Text("SDL3 GPU Samples");
    ImGui::End();
}
} // namespace sample
