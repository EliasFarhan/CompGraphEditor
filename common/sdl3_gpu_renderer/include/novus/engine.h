#pragma once

#include "engine/engine.h"

namespace novus
{

class Engine: public core::Engine
{
public:
	core::TextureManager& GetTextureManager() override;
    [[nodiscard]] SDL_Window* GetWindow() const
    {
        return window_;
    }

protected:
	void Begin() override;
	void End() override;

	void ResizeWindow(glm::uvec2) override;

	void PreUpdate() override;

	void PreImGuiDraw() override;

	void PostImGuiDraw() override;

	void SwapWindow() override;
};

class TextureManager;

TextureManager& GetTextureManager();
SDL_GPUDevice* GetDevice();
SDL_Window* GetWindow();
SDL_GPUCommandBuffer* GetCommandBuffer();
SDL_GPUTexture* GetSwapchainTexture();

SDL_GPUTextureFormat GetSwapchainTextureFormat();
} // namespace novus
