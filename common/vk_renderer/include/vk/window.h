#pragma once
#include "proto/config.pb.h"
#include <vulkan/vulkan.h>
#include <SDL.h>

#include <vector>
#include <cstdint>

namespace gpr5300::vk
{

struct Driver
{
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    float maxAnisotropy = 0.0f;
};

struct Swapchain
{
    VkSwapchainKHR swapChain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    VkFormat imageFormat;
    VkExtent2D extent;

    std::uint32_t minImageCount = 0;
    std::uint32_t imageCount = 0;
};

class Window
{
public:
    Window(const pb::Config& config);
    void Begin();

    void End();

    void CleanupSwapChain();
    void CreateSwapChainObjects();

    Driver& GetDriver() { return driver_; }
    Swapchain& GetSwapChain() { return swapchain_; }
private:

    void CreateWindow();

    void CreateInstance();

    void SetupDebugMessenger();

    void CreateLogicalDevice();

    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();


    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    SDL_Window* window_ = nullptr;
    Driver driver_;
    Swapchain swapchain_;
    const pb::Config& config_;

    VkDebugUtilsMessengerEXT debugMessenger_;

};

Driver& GetDriver();
Swapchain& GetSwapchain();

}