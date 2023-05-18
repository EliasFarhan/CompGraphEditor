#pragma once

#include "engine/engine.h"
#include "vk/texture.h"
#include "vk/window.h"

#include <volk.h>
#include <vk_mem_alloc.h>

#include "common.h"
#include "imgui_manager.h"

namespace vk
{

struct Renderer
{
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::size_t currentFrame = 0;
    std::uint32_t imageIndex = 0;
    VkCommandPool commandPool;
    /**
     * \brief Should be used with imageIndex
     */
    std::vector<VkCommandBuffer> commandBuffers;

};

class Engine final : public core::Engine
{
public:

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    Engine();
    core::TextureManager &GetTextureManager() override { return textureManager_; }
    Window& GetWindow() { return window_; }

    Renderer& GetRenderer() { return renderer_; }
    VmaAllocator& GetAllocator() { return allocator_; }


    void SetVersion(int major, int minor);
    std::uint32_t GetVulkanVersion() const;

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
    //ImGuiManager imGuiManager_{};
    VmaAllocator allocator_{};

};

Renderer& GetRenderer();
Window& GetWindow();
Engine& GetEngine();
VmaAllocator& GetAllocator();
std::uint32_t GetVulkanVersion();

}
