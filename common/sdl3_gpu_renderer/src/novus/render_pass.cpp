//
// Created by unite on 05.11.2025.
//
#include "novus/render_pass.h"

namespace  novus
{

SDL_GPURenderPass* GenerateSubPass(SDL_GPUCommandBuffer* commandBuffer, const renderer::SubpassT& subpass,
                                   std::span<SDL_GPUTexture*> colorTargets, SDL_GPUTexture* depthStencilTarget)
{
    const auto& subpassInfo = subpass.info;
    std::vector<SDL_GPUColorTargetInfo> sdlColorTargetsInfos;
    sdlColorTargetsInfos.reserve(subpassInfo->color_target_infos.size());
    int index = 0;
    for (auto& colorTargetInfo : subpassInfo->color_target_infos)
    {
        SDL_GPUColorTargetInfo colorTarget{};
        colorTarget.texture = colorTargets[index];
        colorTarget.clear_color = colorTargetInfo.clear_color;
        colorTarget.load_op = static_cast<SDL_GPULoadOp>(colorTargetInfo.load_op);
        colorTarget.store_op = static_cast<SDL_GPUStoreOp>(colorTargetInfo.store_op);
        sdlColorTargetsInfos.push_back(colorTarget);
        index++;
    }
    bool isDepthTargetValid = false;
    const auto& depthStencilTargetInfo = subpassInfo->depth_stencil_target_info;
    SDL_GPUDepthStencilTargetInfo sdlDepthStencilTargetInfo{};
    if (depthStencilTargetInfo != nullptr)
    {
        isDepthTargetValid = true;
        if (depthStencilTarget == nullptr)
        {
            throw std::runtime_error("Render Pass: Missing depth stencil target");
        }
        // TODO add cycle?
        sdlDepthStencilTargetInfo.texture = depthStencilTarget;
        sdlDepthStencilTargetInfo.clear_depth = depthStencilTargetInfo->clear_depth;
        sdlDepthStencilTargetInfo.clear_stencil = depthStencilTargetInfo->clear_stencil;
        sdlDepthStencilTargetInfo.store_op = static_cast<SDL_GPUStoreOp>(depthStencilTargetInfo->store_op);
        sdlDepthStencilTargetInfo.load_op = static_cast<SDL_GPULoadOp>(depthStencilTargetInfo->load_op);
        sdlDepthStencilTargetInfo.stencil_load_op = static_cast<SDL_GPULoadOp>(depthStencilTargetInfo->stencil_load_op);
        sdlDepthStencilTargetInfo.stencil_store_op =
            static_cast<SDL_GPUStoreOp>(depthStencilTargetInfo->stencil_store_op);
    }
    return SDL_BeginGPURenderPass(commandBuffer, sdlColorTargetsInfos.data(), sdlColorTargetsInfos.size(),
                                  isDepthTargetValid ? &sdlDepthStencilTargetInfo : nullptr);
}
} // namespace novus
