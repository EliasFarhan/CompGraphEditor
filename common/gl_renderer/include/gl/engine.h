#pragma once

#include "gl/texture.h"
#include "engine/engine.h"

namespace gpr5300::gl
{

class Engine: public gpr5300::Engine
{
public:
    gl::TextureManager& GetTextureManager() override;
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
    TextureManager textureManager_;
};
}