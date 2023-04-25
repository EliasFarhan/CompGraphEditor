#pragma once

#include "gl/texture.h"
#include "engine/engine.h"

namespace gl
{
struct GlVersion
{
    int major{};
    int minor{};
    bool es{};
};
class Engine: public core::Engine
{
public:
    Engine();
    TextureManager& GetTextureManager() override;
    void SetVersion(int major, int minor, bool es);
    GlVersion GetGlVersion() const;
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

GlVersion GetGlVersion();

}