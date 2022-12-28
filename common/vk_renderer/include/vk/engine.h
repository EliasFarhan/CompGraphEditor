#pragma once

#include "engine/engine.h"

#include "vk/texture.h"
#include "vk/window.h"

#include <vk_mem_alloc.h>

namespace gpr5300::vk
{

class Engine: public gpr5300::Engine
{
public:
    Engine();
    TextureManager &GetTextureManager() override { return textureManager_; }

protected:
    void Begin() override;
    void End() override;
    void ResizeWindow(glm::uvec2 uvec2) override;

    void PreUpdate() override;

    void PreImGuiDraw() override;

    void PostImGuiDraw() override;

    void SwapWindow() override;

private:
    Window window_;
    TextureManager textureManager_;
};

}