#pragma once

#include "gl/texture.h"
#include "engine/engine.h"

namespace gl
{

class Engine: public core::Engine
{
public:
    TextureManager& GetTextureManager() override;
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