#pragma once
#include "engine/filesystem.h"
#include "proto/renderer.pb.h"
#include "common.h"

#include <volk.h>

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

constexpr auto swapchainFormat = VK_FORMAT_R8G8B8A8_SRGB;
constexpr auto swapchainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

constexpr std::array<const char*, 1> deviceExtensions =
        {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
constexpr std::array raytracingDeviceExtensions =
{
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
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
std::optional<VkShaderModule> CreateShaderModule(const core::FileBuffer& bufferFile, VkDevice device);

bool CheckRaytracingExtensionSupport(VkPhysicalDevice device);

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

std::uint64_t GetBufferDeviceAddress(VkBuffer buffer);

VkCommandBuffer BeginSingleTimeCommands();
void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties);
void DestroyBuffer(const Buffer& buffer);
Image CreateImage(uint32_t width, uint32_t height, VkFormat format, std::uint32_t layerCount, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels, VkImageCreateFlags flags = 0);
void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
    std::uint32_t mipLevels, std::uint32_t layerCount, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

void CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, std::size_t bufferSize);
void CopyImageFromBuffer(const Buffer& srcBuffer, const Image& image, int width, int height, int layerCount);

constexpr uint32_t alignedSize(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

}