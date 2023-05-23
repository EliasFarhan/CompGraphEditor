#include "vk/framebuffer.h"

#include "utils/log.h"
#include "vk/engine.h"
#include "vk/utils.h"

#include <fmt/format.h>

#include <ranges>


namespace vk
{

void Framebuffer::Load(const core::pb::FrameBuffer& framebufferPb)
{
    auto& engine = GetEngine();
    auto& driver = GetDriver();
    auto windowSize = core::GetWindowSize();
    name = framebufferPb.name();
    targets.reserve(framebufferPb.color_attachments_size() + (framebufferPb.has_depth_stencil_attachment() ? 1 : 0));
    for(auto& targetInfo: framebufferPb.color_attachments())
    {
        targets.emplace_back();
        auto& renderTarget = targets.back();
        const auto format = GetFramebufferFormat(targetInfo.format(), targetInfo.format_size(), targetInfo.type());
        const auto targetSize = targetInfo.size_type() == core::pb::RenderTarget_Size_WINDOW_SIZE ? windowSize : glm::uvec2(targetInfo.target_size().x(), targetInfo.target_size().y());
        renderTarget.name = targetInfo.name();
        renderTarget.format = format;
        renderTarget.image = CreateImage(targetSize.x,
            targetSize.y,
            format,
            1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
            (targetInfo.rbo() ? 0 : VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            1);
        renderTarget.imageView = CreateImageView(driver.device,
            renderTarget.image.image,
            format,
            VK_IMAGE_ASPECT_COLOR_BIT);
        if(!targetInfo.rbo())
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = driver.maxAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            if (vkCreateSampler(driver.device, &samplerInfo, nullptr, &renderTarget.sampler) != VK_SUCCESS)
            {
                LogError(fmt::format("Could not create sampler for render target color attachment: {}", targetInfo.name()));
            }
        }
    }

    if(framebufferPb.has_depth_stencil_attachment())
    {
        auto& targetInfo = framebufferPb.depth_stencil_attachment();
        targets.emplace_back();
        auto& renderTarget = targets.back();
        renderTarget.name = targetInfo.name();
        const auto format = GetFramebufferFormat(targetInfo.format(), targetInfo.format_size(), targetInfo.type());
        const auto targetSize = targetInfo.size_type() == core::pb::RenderTarget_Size_WINDOW_SIZE ? windowSize : glm::uvec2(targetInfo.target_size().x(), targetInfo.target_size().y());
        renderTarget.format = format;
        renderTarget.image = CreateImage(targetSize.x,
            targetSize.y,
            format,
            1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | (targetInfo.rbo() ? 0 : VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            1);
        renderTarget.imageView = CreateImageView(driver.device,
            renderTarget.image.image,
            format,
            VK_IMAGE_ASPECT_DEPTH_BIT | (targetInfo.format() == core::pb::RenderTarget_Format_DEPTH_STENCIL ? VK_IMAGE_ASPECT_STENCIL_BIT : 0));
        if (!targetInfo.rbo())
        {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.anisotropyEnable = VK_FALSE;
            samplerInfo.maxAnisotropy = driver.maxAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            if (vkCreateSampler(driver.device, &samplerInfo, nullptr, &renderTarget.sampler) != VK_SUCCESS)
            {
                LogError(fmt::format("Could not create sampler for render target depth stencil attachment: {}", targetInfo.name()));
            }
        }
    }
}

void Framebuffer::Destroy()
{
    auto& driver = GetDriver();
    const auto& allocator = GetAllocator();
    for(const auto& target: targets)
    {
        vmaDestroyImage(allocator, target.image.image, target.image.allocation);
        vkDestroyImageView(driver.device, target.imageView, nullptr);
        if(target.sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(driver.device, target.sampler, nullptr);
        }
    }
    targets.clear();
}

RenderTarget& Framebuffer::GetRenderTarget(std::string_view attachmentName)
{
    return *std::ranges::find_if(targets, [&attachmentName](const auto& target)
        {
            return target.name == attachmentName;
        });
}
}
