//
// Created by unite on 05.11.2025.
//

#ifndef NEKO2_RENDER_PASS_H
#define NEKO2_RENDER_PASS_H
#include <SDL3/SDL_gpu.h>
#include <generated/renderer_generated.h>


namespace novus
{
SDL_GPURenderPass* GenerateSubPass(SDL_GPUCommandBuffer* commandBuffer, const renderer::SubpassT& subpass, std::span<SDL_GPUTexture*> colorTargets, SDL_GPUTexture* depthStencilTarget);
}
#endif // NEKO2_RENDER_PASS_H
