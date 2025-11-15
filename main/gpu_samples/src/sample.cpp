#include "sample.h"

#include <generated/renderer_generated.h>
#include <imgui.h>

#include "novus/mesh.h"
#include "novus/scene.h"

namespace sample
{

Sample MakeSample(std::string_view name, const novus::renderer::SceneT& sceneInfo)
{
    return {name.data(), sceneInfo};
}

void SampleBrowser::Begin()
{

    samples_.push_back(MakeSample("scene1", Scene01()));
    samples_.push_back(MakeSample("scene2", Scene02()));
    samples_.push_back(MakeSample("scene3", Scene03()));
    samples_.push_back(MakeSample("scene4", Scene04()));


    for(auto& sample : samples_)
    {
        sample.scene.SetScene(sample.sceneInfo);
    }
    currentIndex_ = 3;
    sceneManager_.LoadScene(&samples_[currentIndex_].scene);
}
void SampleBrowser::Update(float dt)
{
    if (newIndex_ != -1)
    {
        if (currentIndex_ != -1)
        {
            samples_[currentIndex_].scene.UnloadScene();
        }
        currentIndex_ = newIndex_;
        sceneManager_.LoadScene(&samples_[currentIndex_].scene);
        newIndex_ = -1;
    }
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
    if (ImGui::BeginCombo("Sample", samples_[currentIndex_].sceneName.c_str()))
    {
        for (int64_t sampleIndex = 0; std::size(samples_) > sampleIndex; sampleIndex++)
        {
            if (ImGui::Selectable(samples_[sampleIndex].sceneName.c_str(), sampleIndex == currentIndex_))
            {
                newIndex_ = sampleIndex;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}
} // namespace sample
