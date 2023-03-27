#include "vk/engine.h"

#include <imgui.h>

#include "utils/log.h"
#include "vk/utils.h"

#include <fmt/format.h>

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

Image Engine::CreateImage(uint32_t width, uint32_t height, VkFormat format, std::uint32_t layerCount,
                          VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, int mipLevels, VkImageCreateFlags flags)
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
    

    vmaCreateImage(allocator_, &imageInfo, &allocInfo, &image.image, &image.allocation, nullptr);
    return image;
}

void Engine::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
    std::uint32_t mipLevels, std::uint32_t layerCount, VkCommandBuffer commandBuffer)
{
    if(commandBuffer == VK_NULL_HANDLE)
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

void Engine::CopyImageFromBuffer(const Buffer& srcBuffer, const Image& image, int width, int height, int layerCount)
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
