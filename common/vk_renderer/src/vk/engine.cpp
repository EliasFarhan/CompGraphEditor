#include "vk/engine.h"

#include <imgui.h>

#include "utils/log.h"

namespace gpr5300::vk
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

void Engine::Begin()
{
    window_.Begin();

    gpr5300::Engine::Begin();
}

void Engine::End()
{
    gpr5300::Engine::End();

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


    // TODO Generate commands
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

}

void Engine::PostImGuiDraw()
{
}

void Engine::SwapWindow()
{
    if (vkEndCommandBuffer(renderer_.commandBuffers[renderer_.imageIndex]) != VK_SUCCESS)
    {
        LogError("Failed to record command buffer!");
        std::terminate();
    }
    auto& driver = window_.GetDriver();
    auto& swapchain = window_.GetSwapChain();
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
        VkSemaphore signalSemaphores[] = {
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

Window& GetWindow()
{
    return instance->GetWindow();
}

Engine& GetEngine()
{
    return *instance;
}
}
