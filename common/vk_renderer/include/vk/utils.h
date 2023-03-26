#pragma once
#include "engine/filesystem.h"
#include "proto/renderer.pb.h"

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

constexpr VkShaderStageFlagBits GetShaderStage(core::pb::ShaderType type)
{
    auto stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    switch (type)
    {
    case core::pb::VERTEX:
        stage = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case core::pb::FRAGMENT:
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case core::pb::COMPUTE:
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
        break;
    case core::pb::GEOMETRY:
        stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        break;
    case core::pb::TESSELATION_CONTROL:
        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        break;
    case core::pb::TESSELATION_EVAL:
        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        break;
    case core::pb::RAY_ANY_HIT:
        stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        break;
    case core::pb::RAY_CLOSEST_HIT:
        stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        break;
    case core::pb::RAY_GEN:
        stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        break;
    case core::pb::RAY_MISS:
        stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        break;
    case core::pb::RAY_INTERSECTION:
        stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        break;
    case core::pb::RAY_CALL:
        stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        break;
    default:
        break;
    }
    return stage;
}

}