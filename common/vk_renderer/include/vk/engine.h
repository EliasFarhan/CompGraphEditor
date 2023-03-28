#pragma once

#include "engine/engine.h"

#include "vk/texture.h"
#include "vk/window.h"

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
    /**
     * \brief Should be used with imageIndex
     */
    std::vector<VkFramebuffer> framebuffers;
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
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties) const;
    Image CreateImage(uint32_t width, uint32_t height, VkFormat format, std::uint32_t layerCount, VkImageTiling tiling,
                      VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels, VkImageCreateFlags flags = 0);
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   std::uint32_t mipLevels, std::uint32_t layerCount, VkCommandBuffer commandBuffer=VK_NULL_HANDLE);
    Renderer& GetRenderer() { return renderer_; }

    VmaAllocator& GetAllocator() { return allocator_; }
    void CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, std::size_t bufferSize);
    void CopyImageFromBuffer(const Buffer& srcBuffer, const Image& image, int width, int height, int layerCount);
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
    ImGuiManager imGuiManager_{};
    VmaAllocator allocator_;

};

Renderer& GetRenderer();
Window& GetWindow();
Engine& GetEngine();
VmaAllocator& GetAllocator();

}
