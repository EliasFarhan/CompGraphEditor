#include "vk/engine.h"

#include <imgui.h>

#include "utils/log.h"
#include "vk/utils.h"

namespace vk
{

static Engine* instance = nullptr;


Engine::Engine() : window_(config_)
{
    instance = this;
}

VkCommandBuffer Engine::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer_.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (const auto result = vkAllocateCommandBuffers(window_.GetDriver().device, &allocInfo, &commandBuffer); result != VK_SUCCESS)
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

void Engine::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        LogError("Could not record one-time command buffer");
    }


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (const auto result = vkQueueSubmit(window_.GetDriver().graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS)
    {
        CheckError(result);
        LogError("Could not submit one-time command buffer to queue");
    }
    if (const auto result = vkQueueWaitIdle(window_.GetDriver().graphicsQueue); result != VK_SUCCESS)
    {
        CheckError(result);
        LogError("Could not wait for queue of one-time command buffer");
    }
    vkFreeCommandBuffers(window_.GetDriver().device, renderer_.commandPool, 1, &commandBuffer);
}

Buffer Engine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const
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
    if (vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer.buffer,
        &buffer.allocation, nullptr) != VK_SUCCESS)
    {
        LogError("Failed to create a buffer");
        std::terminate();
    }
    return buffer;
}

void Engine::CopyBuffer(const Buffer& srcBuffer, const Buffer& dstBuffer, std::size_t bufferSize)
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

void Engine::Begin()
{
    window_.Begin();
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = window_.GetDriver().physicalDevice;
    allocatorInfo.device = window_.GetDriver().device;
    allocatorInfo.instance = window_.GetDriver().instance;
    vmaCreateAllocator(&allocatorInfo, &allocator_);
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
    core::Engine::Begin();
    imGuiManager_.Begin();
}

void Engine::End()
{
    const auto& driver = GetDriver();
    vkDeviceWaitIdle(driver.device);
    imGuiManager_.End();
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

void Engine::ResizeWindow(glm::uvec2 uvec2)
{

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
    imGuiManager_.PreImGuiDraw();
}

void Engine::PostImGuiDraw()
{
    imGuiManager_.PostImGuiDraw();
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
}
