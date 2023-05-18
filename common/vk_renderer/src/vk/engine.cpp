#include "vk/engine.h"

#include <imgui.h>

#include "utils/log.h"
#include "vk/utils.h"

#include "proto/config.pb.h"
#include <fmt/format.h>

namespace vk
{

static Engine* instance = nullptr;


Engine::Engine() : window_(config_)
{
    instance = this;
    const auto& fileSystem = core::FilesystemLocator::get();

    if (!fileSystem.IsRegularFile(configFilename) || 
        config_.major_version() == 0 || 
        config_.minor_version() < 3)
    {
        config_.set_major_version(1);
        config_.set_minor_version(3);
        config_.set_es(false);
    }
}


void Engine::SetVersion(int major, int minor)
{
    config_.set_major_version(major);
    config_.set_minor_version(minor);
}

std::uint32_t Engine::GetVulkanVersion() const
{
    return VK_MAKE_API_VERSION(0, config_.major_version(), config_.minor_version(), 0);
}

void Engine::Begin()
{
    window_.Begin();
    VmaVulkanFunctions vmaVulkanFunctions{};
    vmaVulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vmaVulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vmaVulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vmaVulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vmaVulkanFunctions.vkCreateImage = vkCreateImage;
    vmaVulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vmaVulkanFunctions.vkDestroyImage = vkDestroyImage;
    vmaVulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vmaVulkanFunctions.vkFreeMemory = vkFreeMemory;
    vmaVulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vmaVulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vmaVulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vmaVulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vmaVulkanFunctions.vkMapMemory = vkMapMemory;
    vmaVulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    vmaVulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

    vmaVulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    vmaVulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
    vmaVulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    vmaVulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
    vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    vmaVulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    vmaVulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_MAKE_API_VERSION(0,1,0,0);
    allocatorInfo.physicalDevice = window_.GetDriver().physicalDevice;
    allocatorInfo.device = window_.GetDriver().device;
    allocatorInfo.instance = window_.GetDriver().instance;
    allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    
    vmaCreateAllocator(&allocatorInfo, &allocator_);

    window_.CreateSwapChainObjects();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    core::Engine::Begin();
    //imGuiManager_.Begin();
}

void Engine::End()
{
    const auto& driver = GetDriver();
    vkDeviceWaitIdle(driver.device);
    window_.CleanupSwapChain();
    //imGuiManager_.End();
    vkFreeCommandBuffers(driver.device, renderer_.commandPool,
        static_cast<uint32_t>(renderer_.commandBuffers.size()),
        renderer_.commandBuffers.data());
    vkDestroyCommandPool(driver.device, renderer_.commandPool, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(driver.device,
            renderer_.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(driver.device,
            renderer_.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(driver.device, renderer_.inFlightFences[i], nullptr);
    }
    core::Engine::End();
    vmaDestroyAllocator(allocator_);
    window_.End();
}

void Engine::ResizeWindow(glm::uvec2 newSize)
{
    const auto& driver = GetDriver();
    vkDeviceWaitIdle(driver.device);
    window_.CleanupSwapChain();
    window_.CreateSwapChainObjects();
}

void Engine::PreUpdate()
{
    auto& driver = window_.GetDriver();
    auto& swapchain = window_.GetSwapChain();
    vkWaitForFences(driver.device, 1,
        &renderer_.inFlightFences[renderer_.currentFrame],
        VK_TRUE, UINT64_MAX);

    vkAcquireNextImageKHR(driver.device, swapchain.swapChain,
        UINT64_MAX,
        renderer_.imageAvailableSemaphores[renderer_.currentFrame],
        VK_NULL_HANDLE,
        &renderer_.imageIndex);
    if (renderer_.imagesInFlight[renderer_.imageIndex] !=
        VK_NULL_HANDLE)
    {
        vkWaitForFences(driver.device, 1,
            &renderer_.imagesInFlight[renderer_.imageIndex],
            VK_TRUE,
            UINT64_MAX);
    }
    renderer_.imagesInFlight[renderer_.imageIndex] = renderer_.inFlightFences[renderer_.currentFrame];
    vkResetFences(driver.device, 1,
        &renderer_.inFlightFences[renderer_.currentFrame]);


    // Generate commands
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(renderer_.commandBuffers[renderer_.imageIndex], &beginInfo) != VK_SUCCESS)
    {
        LogError("Failed to begin recording command buffer!");
        std::terminate();
    }

}

void Engine::PreImGuiDraw()
{
    //imGuiManager_.PreImGuiDraw();
}

void Engine::PostImGuiDraw()
{
    //imGuiManager_.PostImGuiDraw();
}

void Engine::SwapWindow()
{
    if (vkEndCommandBuffer(renderer_.commandBuffers[renderer_.imageIndex]) != VK_SUCCESS)
    {
        LogError("Failed to record command buffer!");
        std::terminate();
    }
    const auto& driver = window_.GetDriver();
    const auto& swapchain = window_.GetSwapChain();
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        const VkSemaphore waitSemaphores[] = { renderer_.imageAvailableSemaphores[renderer_.currentFrame] };
        const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &renderer_.commandBuffers[renderer_.imageIndex];

        const VkSemaphore signalSemaphores[] = { renderer_.renderFinishedSemaphores[renderer_.currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (const auto status = vkQueueSubmit(driver.graphicsQueue, 1, &submitInfo, renderer_.inFlightFences[renderer_.currentFrame]);
            status != VK_SUCCESS)
        {
            LogError("Failed to submit draw command buffer!");
            CheckError(status);
            //std::terminate();
        }
    }
    {
        const VkSemaphore signalSemaphores[] = {
                    renderer_.renderFinishedSemaphores[renderer_.currentFrame]
        };
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapchain.swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &renderer_.imageIndex;
        presentInfo.pResults = nullptr; // Optional
        vkQueuePresentKHR(driver.presentQueue, &presentInfo);
    }

    renderer_.currentFrame =
        (renderer_.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Engine::CreateCommandPool()
{
    auto& driver = window_.GetDriver();
    LogDebug("Create Command Pool");
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(
        driver.physicalDevice,
        driver.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    if (vkCreateCommandPool(driver.device, &poolInfo, nullptr,
        &renderer_.commandPool) !=
        VK_SUCCESS)
    {
        LogError("Failed to create command pool!");
        std::terminate();
    }
}

void Engine::CreateCommandBuffers()
{
    auto& driver = window_.GetDriver();
    auto& swapChain = window_.GetSwapChain();
    LogDebug("Create Command Buffers");
    renderer_.commandBuffers.resize(swapChain.imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer_.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer_.commandBuffers.size());

    if (vkAllocateCommandBuffers(driver.device, &allocInfo,
        renderer_.commandBuffers.data()) !=
        VK_SUCCESS)
    {
        LogError("Failed to allocate command buffers!\n");
        std::terminate();
    }
}

void Engine::CreateSyncObjects()
{
    const auto& driver = window_.GetDriver();
    const auto& swapchain = window_.GetSwapChain();
    LogDebug("Create Sync Objects");
    renderer_.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.imagesInFlight.resize(swapchain.images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(driver.device, &semaphoreInfo, nullptr,
            &renderer_.imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateSemaphore(driver.device, &semaphoreInfo, nullptr,
                &renderer_.renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateFence(driver.device, &fenceInfo, nullptr,
                &renderer_.inFlightFences[i]) != VK_SUCCESS)
        {
            LogError("Failed to create sync objects!");
            std::terminate();
        }
    }
}

Renderer& GetRenderer()
{
    return instance->GetRenderer();
}

Window& GetWindow()
{
    return instance->GetWindow();
}

Engine& GetEngine()
{
    return *instance;
}

VmaAllocator& GetAllocator()
{
    return instance->GetAllocator();
}

std::uint32_t GetVulkanVersion()
{
    return instance->GetVulkanVersion();
}
}
