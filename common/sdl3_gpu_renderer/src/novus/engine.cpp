#include "novus/engine.h"
#include "novus/texture.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

namespace novus
{
namespace
{

TextureManager textureManager;
SDL_GPUDevice* device_ = nullptr;
SDL_GPUCommandBuffer* commandBuffer_ = nullptr;
SDL_GPUTexture* swapchainTexture_ = nullptr;
Engine* instance_;
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

}

void Engine::PreImGuiDraw()
{
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Engine::PostImGuiDraw()
{

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (swapchainTexture_ != nullptr)
    {
        // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, commandBuffer_);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo target_info = {};
        target_info.texture = swapchainTexture_;
        target_info.clear_color = SDL_FColor { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
        target_info.load_op = SDL_GPU_LOADOP_DONT_CARE;
        target_info.store_op = SDL_GPU_STOREOP_STORE;
        target_info.mip_level = 0;
        target_info.layer_or_depth_plane = 0;
        target_info.cycle = false;
        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(commandBuffer_, &target_info, 1, nullptr);

        // Render ImGui
        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, commandBuffer_, render_pass);

        SDL_EndGPURenderPass(render_pass);
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
SDL_GPUTextureFormat GetSwapchainTextureFormat()
{
    return SDL_GetGPUSwapchainTextureFormat(device_, GetWindow());
}
} // namespace novus
