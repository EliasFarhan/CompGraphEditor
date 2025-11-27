#include "novus/framebuffer.h"

#include "engine/engine.h"
#include "novus/texture.h"

namespace novus
{
std::unique_ptr<core::Image> Framebuffer::GetImage(std::string_view attachmentName)
{
    return nullptr;
}
void Framebuffer::Load(const renderer::FramebufferT& framebufferInfo)
{
    const auto windowSize = core::GetWindowSize();
    colorTextures_.reserve(framebufferInfo.color_attachments.size());
    colorTargets_.reserve(framebufferInfo.color_attachments.size());
    for (int64_t colorTargetIndex = 0; colorTargetIndex < std::ssize(framebufferInfo.color_attachments); colorTargetIndex++)
    {
        auto colorTextureInfo = *framebufferInfo.color_attachments[colorTargetIndex].texture_info;
        const auto& colorTargetInfo = *framebufferInfo.color_attachments[colorTargetIndex].target_info;
        if (colorTextureInfo.width  == 0)
        {
            colorTextureInfo.width  = windowSize.x;
        }
        if (colorTextureInfo.height == 0)
        {
            colorTextureInfo.height = windowSize.y;
        }
        SDL_GPUTexture* colorTexture = nullptr;
        const bool isSwapchainFormat = colorTextureInfo.format == (internal::TextureFormat)GetSwapchainTextureFormat();
        const bool isOnlyRenderTarget = colorTextureInfo.usage == internal::TextureUsageFlags_TEXTUREUSAGE_COLOR_TARGET;
        if (isSwapchainFormat && isOnlyRenderTarget)
        {
            colorTexture = GetSwapchainTexture();
        }
        else
        {
            colorTexture = GenerateTexture(colorTextureInfo);
        }
        SDL_GPUSampler* sampler = nullptr;
        if (framebufferInfo.color_attachments[colorTargetIndex].sampler_info != nullptr)
        {
            const auto& samplerInfo = *framebufferInfo.color_attachments[colorTargetIndex].sampler_info;
            sampler = GenerateSampler(samplerInfo);
        }
        colorTextures_.emplace_back(colorTexture, sampler);
        SDL_GPUColorTargetInfo targetInfo{.texture = colorTexture,
            .mip_level = colorTargetInfo.mip_level,
            .layer_or_depth_plane = colorTargetInfo.layer_or_depth_plane,
            .clear_color = colorTargetInfo.clear_color,
            .load_op = (SDL_GPULoadOp)colorTargetInfo.load_op,
            .store_op = (SDL_GPUStoreOp)colorTargetInfo.store_op };
        colorTargets_.emplace_back(targetInfo);

    }
    if (framebufferInfo.depth_stencil_attachment != nullptr)
    {
        auto depthTextureInfo = *framebufferInfo.depth_stencil_attachment->texture_info;
        if (depthTextureInfo.width  == 0)
        {
            depthTextureInfo.width  = windowSize.x;
        }
        if (depthTextureInfo.height == 0)
        {
            depthTextureInfo.height = windowSize.y;
        }
        depthTexture_ = GenerateTexture(depthTextureInfo);
    }
    if (framebufferInfo.depth_stencil_attachment != nullptr)
    {
        const auto& depthStencilTargetInfo = *framebufferInfo.depth_stencil_attachment->target_info;
        depthStencilTargetInfo_ = {.texture = depthTexture_, .clear_depth = depthStencilTargetInfo.clear_depth,
            .load_op = (SDL_GPULoadOp)depthStencilTargetInfo.load_op, .store_op = (SDL_GPUStoreOp)depthStencilTargetInfo.store_op,
            .stencil_load_op = (SDL_GPULoadOp)depthStencilTargetInfo.stencil_load_op,
            .stencil_store_op = (SDL_GPUStoreOp)depthStencilTargetInfo.store_op,
            .clear_stencil = depthStencilTargetInfo.clear_stencil};

    }
}

void Framebuffer::UpdateColorTargetTexture(SDL_GPUTexture* sdl_gpu_texture)
{
    colorTargets_[0].texture = sdl_gpu_texture;
}

void Framebuffer::Clear()
{
    auto device = GetDevice();
    for (auto& colorTexture : colorTextures_)
    {
        if (colorTexture.texture != nullptr)
        {
            SDL_ReleaseGPUTexture(device, colorTexture.texture);
        }
        if (colorTexture.sampler != nullptr)
        {
            SDL_ReleaseGPUSampler(device, colorTexture.sampler);
        }
    }
    colorTextures_.clear();
    if (depthTexture_ != nullptr)
    {
        SDL_ReleaseGPUTexture(device, depthTexture_);
        depthTexture_ = nullptr;
    }
}
} // namespace novus
