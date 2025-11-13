//
// Created by unite on 05.11.2025.
//

#ifndef NEKO2_RENDER_PASS_H
#define NEKO2_RENDER_PASS_H
#include <SDL3/SDL_gpu.h>
#include <generated/renderer_generated.h>

#include "novus/command.h"


namespace novus
{
//TODO will need a render target manager
SDL_GPURenderPass* GenerateSubPass(SDL_GPUCommandBuffer* commandBuffer, const renderer::RenderpassT& subpass, std::span<SDL_GPUTexture*> colorTargets, SDL_GPUTexture* depthStencilTarget);

class Renderpass
{
public:
    explicit Renderpass(const renderer::RenderpassT& renderpass, int subpassIndex, std::span<const renderer::MaterialT> materials);

    std::span<DrawCommand> GetDrawCommands()
    {
        return drawCommands_;
    }
private:
    std::vector<DrawCommand> drawCommands_;
};
}
#endif // NEKO2_RENDER_PASS_H
