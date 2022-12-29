#pragma once

#include "engine/engine.h"

#include "vk/texture.h"
#include "vk/window.h"

#include <vk_mem_alloc.h>

#include "common.h"
#include "imgui_manager.h"

namespace gpr5300::vk
{

struct Renderer
{
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::size_t currentFrame = 0;
    std::uint32_t imageIndex = 0;
    /**
     * \brief Should be used with imageIndex
     */
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    /**
     * \brief Should be used with imageIndex
     */
    std::vector<VkCommandBuffer> commandBuffers;
    //Depth resources
    Image depthImage;
    VkImageView depthImageView;
};

class Engine final : public gpr5300::Engine
{
public:
    Engine();
    TextureManager &GetTextureManager() override { return textureManager_; }
    Window& GetWindow() { return window_; }
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    Renderer& GetRenderer() { return renderer_; }

protected:
    void Begin() override;
    void End() override;
    void ResizeWindow(glm::uvec2 uvec2) override;

    void PreUpdate() override;

    void PreImGuiDraw() override;

    void PostImGuiDraw() override;

    void SwapWindow() override;

    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();

private:
    Window window_;
    TextureManager textureManager_;
    Renderer renderer_;
    ImGuiManager imGuiManager_;


    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};

Renderer& GetRenderer();
Window& GetWindow();
Engine& GetEngine();

}
