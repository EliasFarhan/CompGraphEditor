#include "novus/engine.h"

namespace novus
{

constexpr SDL_GPUShaderFormat ConvertShaderFormat(novus::engine::ShaderFormat shaderFormat)
{
	switch(shaderFormat)
	{
	case engine::ShaderFormat_SPIRV:
		return SDL_GPU_SHADERFORMAT_SPIRV;
		break;
	case engine::ShaderFormat_DXIL:
		return SDL_GPU_SHADERFORMAT_DXIL;
		break;
	case engine::ShaderFormat_MSL:
		return SDL_GPU_SHADERFORMAT_MSL;
		break;
	case engine::ShaderFormat_METALLIB:
		return SDL_GPU_SHADERFORMAT_METALLIB;
		break;
	case engine::ShaderFormat_DXBC:
		return SDL_GPU_SHADERFORMAT_DXBC;
		break;
	}
	return SDL_GPU_SHADERFORMAT_INVALID;
}
core::TextureManager& Engine::GetTextureManager()
{
}

void Engine::Begin()
{

#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
	float mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	const auto windowSize = glm::ivec2(config_.window_size.x, config_.window_size.y);
	window_ = SDL_CreateWindow(
		config_.window_name.c_str(),
		windowSize.x,
		windowSize.y,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	if (window_ == nullptr)
	{
		throw std::runtime_error(std::format("Failed to create window. SDL Error: {}", SDL_GetError()));
	}
	device_ = SDL_CreateGPUDevice(ConvertShaderFormat(config_.shader_format), true, nullptr);
	if (!device_)
	{
		throw std::runtime_error(std::format("Failed to create GPU device. Error: {}", SDL_GetError()));
	}

	// Assign the GPU device to the window
	if (!SDL_ClaimWindowForGPUDevice(device_, window_))
	{
		throw std::runtime_error(std::format("Failed to claim window for GPU device. Error: {}", SDL_GetError()));
	}
	core::Engine::Begin();
}

void Engine::End()
{
	core::Engine::End();
}

void Engine::ResizeWindow(glm::uvec2)
{
}

void Engine::PreUpdate()
{
}

void Engine::PreImGuiDraw()
{
}

void Engine::PostImGuiDraw()
{
}

void Engine::SwapWindow()
{
}
}
