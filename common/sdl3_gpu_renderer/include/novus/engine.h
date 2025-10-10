#pragma once
#include "engine/engine.h"
#include "novus/texture.h"

namespace novus
{

class Engine: public core::Engine
{
public:
	core::TextureManager& GetTextureManager() override;

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
}