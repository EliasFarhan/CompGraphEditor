#include "vk/window.h"
#include "utils/log.h"
#include "vk/engine.h"
#include "vk/utils.h"

#include <SDL3/SDL_vulkan.h>
#include <format>


namespace vk
{
static Window* instance = nullptr;

Window::Window(const novus::engine::ConfigT& config) : config_(config) { instance = this; }

void Window::Begin()
{
    CreateWindow();
    CreateInstance();
    CreateSurface();
    CreateLogicalDevice();
}

void Window::End()
{
    LogDebug("Destroy Window");
    vkb::destroy_device(driver_.vkbDevice);
    vkDestroySurfaceKHR(driver_.instance, driver_.surface, nullptr);

    //vkb::destroy_debug_utils_messenger(driver_.instance, driver_.instance.debug_messenger);
    vkb::destroy_instance(driver_.instance);
}

void Window::CreateWindow()
{
    LogDebug("Creating SDL window with Vulkan enabled");
    int windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    if (config_.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    window_ = SDL_CreateWindow(config_.window_name.c_str(), config_.window_size.x, config_.window_size.y,
                               windowFlags);
    if (!window_)
    {
        LogError("Could not create SDL window");
        std::terminate();
    }
}

void Window::CreateInstance()
{
    if (volkInitialize() != VK_SUCCESS)
    {
        LogError("Could not initialize volk!");
        std::terminate();
    }

    LogDebug("Creating Instance");
    vkb::InstanceBuilder builder;

    std::array<const char*, 2> additionalExtensions = {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME, // example additional extension
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME // adding validation layers
    };
    builder.set_app_name(config_.window_name.c_str())
        .set_engine_name("Neko3d engine")
        .require_api_version(config_.major_version, config_.minor_version)
        .request_validation_layers(config_.enable_debug)
        .enable_extensions(additionalExtensions.size(), additionalExtensions.data());
    builder.set_debug_callback(
        [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) -> VkBool32
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            {
                LogError(std::format("[{}: {}] {}\n", vkb::to_string_message_severity(messageSeverity),
                                     vkb::to_string_message_type(messageType), pCallbackData->pMessage));
            }
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            {
                LogWarning(std::format("[{}: {}] {}\n", vkb::to_string_message_severity(messageSeverity),
                                       vkb::to_string_message_type(messageType), pCallbackData->pMessage));
            }
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            {
                LogDebug(std::format("[{}: {}] {}\n", vkb::to_string_message_severity(messageSeverity),
                                     vkb::to_string_message_type(messageType), pCallbackData->pMessage));
            }
            // Return false to move on, but return true for validation to skip passing down the call to the driver
            return VK_TRUE;
        });
    auto instRet = builder.build();
    if (!instRet.has_value())
    {
        LogError("Could not create instance!");
        std::terminate();
    }
    driver_.instance = instRet.value();

    volkLoadInstanceOnly(driver_.instance);
}


void Window::CreateLogicalDevice()
{
    LogDebug("[Log] Creating Logical Device");

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vkb::PhysicalDeviceSelector selector{driver_.instance};
    auto physRet = selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
                       .require_present()
                       .set_surface(driver_.surface)
                        .add_required_extension("VK_EXT_extended_dynamic_state")
                       .set_required_features(deviceFeatures)
                        .set_minimum_version(config_.major_version, config_.minor_version)
                       .select();
    if (!physRet)
    {
        LogError(std::format("Failed to select Vulkan Physical Device. Error: {}", physRet.error().message()));
        std::terminate();
    }
    driver_.physicalDevice = physRet.value().physical_device;


    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(driver_.physicalDevice, &properties);
    driver_.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    // Checking if raytracing features can be enabled
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR addressFeaturesKhr{};
    addressFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    addressFeaturesKhr.bufferDeviceAddress = VK_TRUE;


    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKhr{};
    accelerationStructureFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureFeaturesKhr.accelerationStructure = VK_TRUE;


    VkPhysicalDeviceRayTracingPipelineFeaturesKHR pipelineFeaturesKhr{};
    pipelineFeaturesKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    pipelineFeaturesKhr.rayTracingPipeline = VK_TRUE;

    const bool hasPipelineFeaturesKhr = physRet->enable_extension_features_if_present(pipelineFeaturesKhr);
    const bool hasAddressFeaturesKhr = physRet->enable_extension_features_if_present(addressFeaturesKhr);
    const bool hasAccelerationStructureFeaturesKhr =
        physRet->enable_extension_features_if_present(accelerationStructureFeaturesKhr);

    std::vector raytracingDeviceExtensions(vk::raytracingDeviceExtensions.begin(),
                                           vk::raytracingDeviceExtensions.end());
    const bool hasRaytracingExtensions = physRet->enable_extensions_if_present(raytracingDeviceExtensions);
    if (hasAccelerationStructureFeaturesKhr && hasAddressFeaturesKhr && hasPipelineFeaturesKhr &&
        hasRaytracingExtensions)
    {
        hasRaytracing_ = true;

        rayTracingPipelineProperties_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        VkPhysicalDeviceProperties2 deviceProps2 = {};
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProps2.pNext = &rayTracingPipelineProperties_;

        vkGetPhysicalDeviceProperties2(driver_.physicalDevice, &deviceProps2);

        accelerationStructureFeatures_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &accelerationStructureFeatures_;
        vkGetPhysicalDeviceFeatures2(driver_.physicalDevice, &deviceFeatures2);
    }


    vkb::DeviceBuilder deviceBuilder{physRet.value()};
    auto deviceRet = deviceBuilder.build();
    if (!deviceRet)
    {
        LogError(std::format("Failed to create Vulkan device. Error: {}", deviceRet.error().message()));
        std::terminate();
    }
    driver_.vkbDevice = deviceRet.value();
    driver_.device = deviceRet.value().device;

    auto graphicsQueue = driver_.vkbDevice.get_queue(vkb::QueueType::graphics);
    if (!graphicsQueue.has_value())
    {
        LogError("[Error] Failed to create graphics queue!");
        std::terminate();
    }
    driver_.graphicsQueue = graphicsQueue.value();
    auto presentQueue = driver_.vkbDevice.get_queue(vkb::QueueType::present);
    if (!presentQueue.has_value())
    {
        LogError("[Error] Failed to create present queue!");
        std::terminate();
    }
    driver_.presentQueue = presentQueue.value();
    volkLoadDevice(driver_.device);
}

void Window::CreateSurface()
{
    LogDebug("Creating Surface");
    if (!SDL_Vulkan_CreateSurface(window_, driver_.instance.instance, nullptr, &driver_.surface))
    {
        LogError("[Vulkan] Failed to create a window surface!");
        std::terminate();
    }
}

void Window::CreateSwapchain()
{
    LogDebug("[Vulkan] Create SwapChain");
    vkb::SwapchainBuilder swapchainBuilder{driver_.vkbDevice};
    auto swapchainRet = swapchainBuilder
        .set_desired_min_image_count(2)
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_old_swapchain(swapchain_.vkbSwapchain)
        .build();
    if (!swapchainRet)
    {
        throw std::runtime_error(std::format("{} {}", swapchainRet.error().message(), (int)swapchainRet.vk_result()));
    }
    vkb::destroy_swapchain(swapchain_.vkbSwapchain);
    swapchain_.vkbSwapchain = swapchainRet.value();
    swapchain_.swapchain = swapchain_.vkbSwapchain.swapchain;
    swapchain_.imageCount = swapchain_.vkbSwapchain.image_count;
    swapchain_.imageFormat = swapchain_.vkbSwapchain.image_format;
    swapchain_.extent = swapchain_.vkbSwapchain.extent;
    swapchain_.minImageCount = swapchain_.vkbSwapchain.requested_min_image_count;
    swapchain_.images = swapchain_.vkbSwapchain.get_images().value();


    LogDebug(
        std::format("[Vulkan] Image count: {}, min image count: {}", swapchain_.imageCount, swapchain_.minImageCount));
}

void Window::CreateImageViews()
{
    LogDebug("Create Image Views");
    swapchain_.imageViews = swapchain_.vkbSwapchain.get_image_views().value();
}

void Window::CreateDepthResources()
{
    const VkFormat depthFormat = FindDepthFormat(driver_.physicalDevice);
    swapchain_.depthImage =
        CreateImage(swapchain_.extent.width, swapchain_.extent.height, depthFormat, 1, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1);

    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapchain_.depthImage.image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = depthFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(driver_.device, &createInfo, nullptr, &swapchain_.depthImageView) != VK_SUCCESS)
    {
        LogError("[Error] Failed to create image views!");
        std::terminate();
    }
}

void Window::CleanupSwapChain()
{
    LogDebug("Cleanup Swapchain");
    vkDestroyImageView(driver_.device, swapchain_.depthImageView, nullptr);
    vmaDestroyImage(GetAllocator(), swapchain_.depthImage.image, swapchain_.depthImage.allocation);

    swapchain_.vkbSwapchain.destroy_image_views(swapchain_.imageViews);
    vkb::destroy_swapchain(swapchain_.vkbSwapchain);
}

void Window::CreateSwapChainObjects()
{
    CreateSwapchain();
    CreateImageViews();
    CreateDepthResources();
}

VkExtent2D Window::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    SDL_GetWindowSize(window_, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    actualExtent.width =
        std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
        std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

Driver& GetDriver() { return instance->GetDriver(); }

Swapchain& GetSwapchain() { return instance->GetSwapChain(); }

bool HasRaytracing() { return instance->HasRaytracing(); }

VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties()
{
    return instance->GetRayTracingPipelineProperties();
}
} // namespace vk
