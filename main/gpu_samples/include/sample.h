//
// Created by unite on 05.11.2025.
//

#ifndef NEKO2_SAMPLE_BROWSER_H
#define NEKO2_SAMPLE_BROWSER_H
#include "engine/engine.h"

namespace sample
{


class SampleBrowser: public core::OnEventInterface, public core::OnGuiInterface, public core::System
{
public:
    void Begin() override;
    void Update(float dt) override;
    void End() override;
    void OnEvent(SDL_Event& event) override;
    void OnGui() override;
};

}


#endif // NEKO2_SAMPLE_BROWSER_H
