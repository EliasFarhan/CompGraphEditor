#include "vk/window.h"
#include "vk/utils.h"
#include "utils/log.h"

#include <fmt/core.h>
#include <SDL_vulkan.h>

namespace gpr5300::vk
{
static Window* instance = nullptr;

Window::Window(const pb::Config& config) : config_(config)
{
    instance = this;
}

void Window::Begin()
{
    CreateWindow();
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    driver_.physicalDevice = PickPhysicalDevice(driver_.instance, driver_.surface);
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(driver_.physicalDevice, &properties);
    driver_.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    CreateLogicalDevice();
    CreateSwapChainObjects();
}

void Window::End() {

}

void Window::CreateWindow() {
    LogDebug("Creating SDL window with Vulkan enabled");
    int windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    if(config_.fullscreen())
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    window_ = SDL_CreateWindow(
            config_.window_name().c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            config_.windowsize().x(),
            config_.windowsize().y(),
            windowFlags
    );
    if(!window_)
    {
        LogError("Could not create SDL window");
        std::terminate();
    }
}

void Window::CreateInstance()
{
    LogDebug("Creating Instance");
    if (config_.enable_debug() && !CheckValidationLayerSupport())
    {
        LogError("Validation layers requested, but not available!");
        std::terminate();
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = config_.window_name().c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "GPR5300 engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr))
    {
        LogError("[Error] SDL Vulkan, Could not get extensions count");
        std::terminate();
    }

    const char* const additionalExtensions[] =
            {
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, // example additional extension
                    VK_EXT_DEBUG_UTILS_EXTENSION_NAME //adding validation layers
            };
    const size_t additionalExtensionsCount =
            sizeof(additionalExtensions) / sizeof(additionalExtensions[0]);
    const size_t extensionCount = count + additionalExtensionsCount;
    std::vector<const char*> extensionNames(extensionCount);

    // get names of required extensions
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, &extensionNames[0]))
    {
        LogError("SDL Vulkan, Cannot get instance extensions");
        std::terminate();
    }

    // copy additional extensions after required extensions
    for (size_t i = 0; i < additionalExtensionsCount; i++)
    {
        extensionNames[i + count] = additionalExtensions[i];
    }
    SDL_Vulkan_GetInstanceExtensions(window_, &count, &extensionNames[0]);

    LogDebug("Vulkan extensions:");
    for (auto& extension : extensionNames)
    {
        LogDebug(extension);
    }

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (config_.enable_debug())
    {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        instanceCreateInfo.pNext = &debugCreateInfo;

    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &driver_.instance) != VK_SUCCESS)
    {
        LogError("Failed to create instance!\n");
        std::terminate();
    }

}

void Window::SetupDebugMessenger()
{
    if (!config_.enable_debug())
    {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(driver_.instance, &createInfo, nullptr,
                                     &debugMessenger_) !=
        VK_SUCCESS)
    {
        LogWarning("Failed to set up debug messenger!");
    }
}

void Window::CreateLogicalDevice() {
    LogDebug("[Log] Creating Logical Device");
    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice, driver_.surface);


    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (config_.enable_debug())
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(driver_.physicalDevice, &createInfo, nullptr, &driver_.device) != VK_SUCCESS)
    {
        LogError("[Error] Failed to create logical device!");
        std::terminate();
    }
    vkGetDeviceQueue(driver_.device, indices.graphicsFamily.value(), 0, &driver_.graphicsQueue);
    vkGetDeviceQueue(driver_.device, indices.presentFamily.value(), 0, &driver_.presentQueue);
}

void Window::CreateSurface() {
    LogDebug("Creating Surface");
    if (!SDL_Vulkan_CreateSurface(window_, driver_.instance, &driver_.surface))
    {
        LogError("[Vulkan] Failed to create a window surface!");
        std::terminate();
    }
}

void Window::CreateSwapChain()
{
    LogDebug("[Vulkan] Create SwapChain");
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(driver_.physicalDevice, driver_.surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    // We choose double buffering
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
    LogDebug(fmt::format("[Vulkan] Swapchain support, minImageCount: {} maxImageCount: {}",
                               swapChainSupport.capabilities.minImageCount,
                               swapChainSupport.capabilities.maxImageCount));
    swapchain_.minImageCount = swapChainSupport.capabilities.minImageCount;
    //Weird hack to make ImGui work on AMD? Because it requires minImageCount to be minimum 2...
    if (swapchain_.minImageCount < 2 && swapChainSupport.capabilities.maxImageCount >= 3)
    {
        swapchain_.minImageCount = 2;
    }
    swapchain_.imageCount = swapchain_.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && swapchain_.imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        swapchain_.imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    LogDebug(fmt::format("[Vulkan] Image count: {}, min image count: {}", swapchain_.imageCount, swapchain_.minImageCount));

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = driver_.surface;

    createInfo.minImageCount = swapchain_.imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice, driver_.surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(driver_.device, &createInfo, nullptr, &swapchain_.swapChain) != VK_SUCCESS)
    {
        LogError("Failed to create swap chain!");
        std::terminate();
    }
    //Adding images
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain, &swapchain_.imageCount, nullptr);
    swapchain_.images.resize(swapchain_.imageCount);
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain, &swapchain_.imageCount, swapchain_.images.data());

    swapchain_.imageFormat = surfaceFormat.format;
    swapchain_.extent = extent;
}

void Window::CreateImageViews()
{
    LogDebug("Create Image Views");
    swapchain_.imageViews.resize(swapchain_.images.size());
    for (size_t i = 0; i < swapchain_.images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchain_.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain_.imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(driver_.device, &createInfo, nullptr, &swapchain_.imageViews[i]) != VK_SUCCESS)
        {
            LogError("[Error] Failed to create image views!");
            std::terminate();
        }
    }
}

void Window::CleanupSwapChain() {

}

void Window::CreateSwapChainObjects() {

}

VkExtent2D Window::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    SDL_GetWindowSize(window_, &width, &height);

    VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

Driver& GetDriver()
{
    return instance->GetDriver();
}
}
