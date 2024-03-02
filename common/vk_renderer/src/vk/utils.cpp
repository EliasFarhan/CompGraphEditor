#include "utils/log.h"
#include "vk/utils.h"

#include <fmt/core.h>

#include <set>
#include <cstring>
#include <vk/window.h>
#include <vk/engine.h>

#include "vk/common.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace vk
{

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void* pUserData)
{
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        return VK_FALSE;
    const auto msg = pCallbackData->pMessage;
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LogError(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LogWarning(msg);
        break;
    default:
        LogDebug(msg);
        break;
    }
    return VK_FALSE;
}

bool CheckValidationLayerSupport()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    std::uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    LogDebug("Available layers: ");
    for (auto& layerProperties : availableLayers)
    {
        LogDebug(layerProperties.layerName);
    }

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (std::strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{

    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    createInfo.pUserData = nullptr;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto func = GetFuncPointer<PFN_vkCreateDebugUtilsMessengerEXT>
            (instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        LogWarning("Debug Utils Messenger EXT is null");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    // Assign index to queue families that could be found
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (indices.IsComplete())
            break;
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) //Checking if available queue family supports graphics command.
        {
            indices.graphicsFamily = i;
        }
        i++;
    }
    return indices;
}

bool CheckRaytracingExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> raytracingExtensions(raytracingDeviceExtensions.cbegin(), raytracingDeviceExtensions.cend());

    for (const auto& extension : availableExtensions)
    {
        raytracingExtensions.erase(extension.extensionName);
    }

    return raytracingExtensions.empty();
}

std::uint64_t GetBufferDeviceAddress(VkBuffer buffer)
{
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;
    return vkGetBufferDeviceAddressKHR(GetDriver().device, &bufferDeviceAI);
}

VkCommandBuffer BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = GetRenderer().commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (const auto result = vkAllocateCommandBuffers(GetDriver().device, &allocInfo, &commandBuffer); result != VK_SUCCESS)
    {
        LogError("Could not allocate one-time command buffer");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (const auto result = vkBeginCommandBuffer(commandBuffer, &beginInfo); result != VK_SUCCESS)
    {
        LogError("Could not start one-time command buffer");
    }


    return commandBuffer;
}

void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        LogError("Could not record one-time command buffer");
    }


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (const auto result = vkQueueSubmit(GetDriver().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS)
    {
        CheckError(result);
        LogError("Could not submit one-time command buffer to queue");
    }
    if (const auto result = vkQueueWaitIdle(GetDriver().graphicsQueue); result != VK_SUCCESS)
    {
        CheckError(result);
        LogError("Could not wait for queue of one-time command buffer");
    }
    vkFreeCommandBuffers(GetDriver().device, GetRenderer().commandPool, 1, &commandBuffer);
}

Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    Buffer buffer{};
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    allocInfo.requiredFlags = properties;
    if (vmaCreateBuffer(GetAllocator(), &bufferInfo, &allocInfo, &buffer.buffer,
        &buffer.allocation, nullptr) != VK_SUCCESS)
    {
        LogError("Failed to create a buffer");
        std::terminate();
    }
    return buffer;
}

void DestroyBuffer(const Buffer& buffer)
{
    vmaDestroyBuffer(GetAllocator(), buffer.buffer, buffer.allocation);
}

Image CreateImage(uint32_t width, uint32_t height, VkFormat format, std::uint32_t layerCount, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels, VkImageCreateFlags flags)
{
    Image image;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layerCount;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = flags;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = properties;


    vmaCreateImage(GetAllocator(), &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr);
    return image;
}

void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, std::uint32_t mipLevels,
    std::uint32_t layerCount, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        const VkCommandBuffer commandBufferTmp = BeginSingleTimeCommands();

        TransitionImageLayout(image, oldLayout, newLayout, mipLevels, layerCount, commandBufferTmp);

        EndSingleTimeCommands(commandBufferTmp);
        return;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;
    constexpr VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    constexpr VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    // Source layouts (old)
            // Source access mask controls actions that have to be finished on the old layout
            // before it will be transitioned to the new layout
    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        barrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (barrier.srcAccessMask == 0)
        {
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_GENERAL:
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, std::size_t bufferSize)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1,
            &copyRegion);
    }
    EndSingleTimeCommands(commandBuffer);
}

void CopyImageFromBuffer(const Buffer& srcBuffer, const Image& image, int width, int height, int layerCount)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    {

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount; //TODO how about cubemap?

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height),
            1
        };
        LogDebug(fmt::format("Region: width: {} height: {} layer count: {}", width, height, layerCount));

        vkCmdCopyBufferToImage(
            commandBuffer,
            srcBuffer.buffer,
            image.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }
    EndSingleTimeCommands(commandBuffer);
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.cbegin(), deviceExtensions.cend());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}



SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    std::uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    std::uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices indices = FindQueueFamilies(device, surface);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);

        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader && indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

int RateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    int score = 0;
    //Checking queue family and geometry shader
    if (!IsDeviceSuitable(device, surface))
        return 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }
    if(CheckRaytracingExtensionSupport(device))
    {
        score += 500;
    }
    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    return score;

}
VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    LogDebug("[Log] Picking a physical device");
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        LogDebug("[Vulkan] Failed to find GPUs with Vulkan support!");
        std::terminate();
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    int maxScore = 0;
    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceInfo;
        vkGetPhysicalDeviceProperties(device, &deviceInfo);
        LogDebug(fmt::format("Device info: {}", deviceInfo.deviceName));
        const int deviceScore = RateDeviceSuitability(device, surface);
        if (deviceScore > maxScore)
        {
            physicalDevice = device;
            maxScore = deviceScore;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        LogDebug("[Vulkan] Failed to find a suitable GPU!");
        std::terminate();
    }
    return physicalDevice;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == swapchainFormat && availableFormat.colorSpace == swapchainColorSpace)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    LogDebug("Destroy Debug Messenger");
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}
VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    LogError("Failed to find supported format!");
    std::terminate();
}
VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    return FindSupportedFormat(physicalDevice,
                               { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        LogError("Failed to create texture image view!");
        std::terminate();
    }

    return imageView;
}

std::optional<VkShaderModule> CreateShaderModule(const core::BufferFile& bufferFile, VkDevice device)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bufferFile.length;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bufferFile.data);
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        LogError("Failed to create shader module!");
        return std::nullopt;
    }
    return shaderModule;
}
}