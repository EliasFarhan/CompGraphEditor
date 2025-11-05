#pragma once
#include "engine/engine.h"
#include "novus/texture.h"

namespace novus
{

class Engine: public core::Engine
{
public:
	core::TextureManager& GetTextureManager() override;

    SDL_GPUDevice* GetDevice() const {
        return device_;
    }
    SDL_Window* GetWindow() const
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

private:
	TextureManager textureManager;
	SDL_GPUDevice* device_ = nullptr;
};

SDL_GPUDevice* GetDevice();
SDL_Window* GetWindow();
} // namespace novus
