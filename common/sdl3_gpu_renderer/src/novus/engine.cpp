#include "novus/engine.h"
#include "novus/texture.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace novus
{
namespace
{
TextureManager textureManager;
SDL_GPUDevice* device_ = nullptr;
SDL_GPUCommandBuffer* commandBuffer_ = nullptr;
SDL_GPUTexture* swapchainTexture_ = nullptr;
Engine* instance_;
bool hasDrawnSomething_ = false;
}

constexpr SDL_GPUShaderFormat ConvertShaderFormat(engine::ShaderFormat shaderFormat)
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
    return textureManager;
}
void Engine::Begin()
{

#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
    instance_ = this;
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
    const auto shaderFormat = ConvertShaderFormat(config_.shader_format);
	device_ = SDL_CreateGPUDevice(shaderFormat, true, nullptr);
	if (!device_)
	{
		throw std::runtime_error(std::format("Failed to create GPU device. Error: {}", SDL_GetError()));
	}

	// Assign the GPU device to the window
	if (!SDL_ClaimWindowForGPUDevice(device_, window_))
	{
		throw std::runtime_error(std::format("Failed to claim window for GPU device. Error: {}", SDL_GetError()));
	}

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(mainScale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLGPU(window_);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = device_;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device_, window_);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.

    ImGui_ImplSDLGPU3_Init(&init_info);

	core::Engine::Begin();
}

void Engine::End()
{
	core::Engine::End();
    SDL_WaitForGPUIdle(device_);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(device_, window_);
    SDL_DestroyGPUDevice(device_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Engine::ResizeWindow(glm::uvec2)
{
}

void Engine::PreUpdate()
{
    commandBuffer_ = SDL_AcquireGPUCommandBuffer(device_);
    Uint32 swapchainWidth = 0, swapchainHeight = 0;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer_, window_, &swapchainTexture_, &swapchainWidth, &swapchainHeight);
    hasDrawnSomething_ = false;
}

void Engine::PreImGuiDraw()
{
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Engine::PostImGuiDraw()
{

    ImVec4 clearColor = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (swapchainTexture_ != nullptr)
    {
        // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        Imgui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer_);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo targetInfo = {};
        targetInfo.texture = swapchainTexture_;
        targetInfo.clear_color = SDL_FColor { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
        targetInfo.load_op = hasDrawnSomething_ ? SDL_GPU_LOADOP_DONT_CARE : SDL_GPU_LOADOP_CLEAR;
        targetInfo.store_op = SDL_GPU_STOREOP_STORE;
        targetInfo.mip_level = 0;
        targetInfo.layer_or_depth_plane = 0;
        targetInfo.cycle = false;
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer_, &targetInfo, 1, nullptr);

        // Render ImGui
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer_, renderPass);

        SDL_EndGPURenderPass(renderPass);
    }

}

void Engine::SwapWindow()
{
    SDL_SubmitGPUCommandBuffer(commandBuffer_);
}
TextureManager& GetTextureManager()
{
    return textureManager;
}

SDL_GPUDevice* GetDevice()
{
    return device_;
}
SDL_Window* GetWindow()
{
    return instance_->GetWindow();
}
SDL_GPUCommandBuffer* GetCommandBuffer()
{
    return commandBuffer_;
}
SDL_GPUTexture* GetSwapchainTexture()
{
    return swapchainTexture_;
}
void EnableDrawSomething()
{
    hasDrawnSomething_ = true;
}
SDL_GPUTextureFormat GetSwapchainTextureFormat()
{
    return SDL_GetGPUSwapchainTextureFormat(device_, GetWindow());
}
} // namespace novus
