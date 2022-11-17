#pragma once

#include "engine/engine.h"

namespace gpr5300::gl
{

class Engine: public gpr5300::Engine
{
protected:
    void Begin() override;
    void End() override;
    void ResizeWindow(glm::uvec2 windowSize) override;
    void PreUpdate() override;
    void PreImGuiDraw() override;
    void PostImGuiDraw() override;
    void SwapWindow() override;
private:
    SDL_GLContext glRenderContext_{};
};
}