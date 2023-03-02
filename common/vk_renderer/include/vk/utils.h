#pragma once
#include "engine/filesystem.h"

#include <vulkan/vulkan.h>

#include <array>
#include <optional>
#include <vector>
#include <string_view>
namespace vk
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool IsComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

constexpr std::array<const char*, 1> deviceExtensions =
        {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

bool CheckValidationLayerSupport();
void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);
VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
/**
 * \brief Function used to query swapchain support when creating a Vulkan or to choose suitable physical device
 */
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
std::optional<VkShaderModule> CreateShaderModule(const core::BufferFile& bufferFile, VkDevice device);


template<typename T>
T GetFuncPointer(VkInstance instance, std::string_view name)
{
    return reinterpret_cast<T>(vkGetInstanceProcAddr(instance,
                                                     name.data()));
}

template<typename T>
T GetFuncPointer(VkDevice device, std::string_view name)
{
    return reinterpret_cast<T>(vkGetDeviceProcAddr(device,
                                                   name.data()));
}
}