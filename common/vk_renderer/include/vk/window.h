#pragma once
#include "proto/config.pb.h"
#include <volk.h>

#include <VkBootstrap.h>
#include <SDL3/SDL.h>

#include <cstdint>
#include <generated/engine_generated.h>
#include <vector>

#include "vk/common.h"

namespace vk
{

struct Driver
{
    vkb::Instance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    vkb::Device vkbDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    float maxAnisotropy = 0.0f;
};

struct Swapchain
{
    vkb::Swapchain vkbSwapchain;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    //Depth resources
    Image depthImage;
    VkImageView depthImageView;

    VkFormat imageFormat;
    VkExtent2D extent;

    std::uint32_t minImageCount = 0;
    std::uint32_t imageCount = 0;
};

class Window
{
public:
    Window(const novus::engine::ConfigT& config);
    void Begin();

    void End();

    void CleanupSwapChain();
    void CreateSwapChainObjects();

    Driver& GetDriver() { return driver_; }
    Swapchain& GetSwapChain() { return swapchain_; }
    SDL_Window* GetSdlWindow() const { return window_; }
    bool HasRaytracing() const { return hasRaytracing_; }
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties() const
    {
        return rayTracingPipelineProperties_;
    }

private:

    void CreateWindow();

    void CreateInstance();


    void CreateLogicalDevice();

    void CreateSurface();
    void CreateSwapchain();
    void CreateImageViews();

    void CreateDepthResources();

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    SDL_Window* window_ = nullptr;
    vkb::InstanceBuilder builder_;
    Driver driver_;
    Swapchain swapchain_;
    const novus::engine::ConfigT& config_;

    VkDebugUtilsMessengerEXT debugMessenger_;

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties_{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures_{};
    bool hasRaytracing_ = false;

};

Driver& GetDriver();
Swapchain& GetSwapchain();
bool HasRaytracing();
VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties();

}
