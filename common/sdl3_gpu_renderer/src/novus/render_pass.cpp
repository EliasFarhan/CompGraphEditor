//
// Created by unite on 05.11.2025.
//
#include "novus/render_pass.h"

#include "novus/framebuffer.h"

namespace  novus
{

SDL_GPURenderPass* GenerateSubPass(SDL_GPUCommandBuffer* commandBuffer, const renderer::RenderpassT& subpass,
                                   const Framebuffer& framebuffer)
{
    const auto colorTargets = framebuffer.GetColorTargets();
    return SDL_BeginGPURenderPass(commandBuffer,
        colorTargets.data(),
        colorTargets.size(),
        framebuffer.GetDepthStencilTargetInfo());
}
Renderpass::Renderpass(const renderer::RenderpassT& renderpass,
    int subpassIndex,
    std::span<Pipeline> pipelines,
    std::span<Material> materials)
{
    drawCommands_.reserve(renderpass.commands.size());
    for (const auto& command : renderpass.commands)
    {
        auto* material = &materials[command.material_index];
        auto* pipeline = &pipelines[material->GetPipelineIndex()];
        drawCommands_.emplace_back(command, subpassIndex, pipeline, material);
    }
    std::ranges::sort(drawCommands_, [&](const DrawCommand& command1, const DrawCommand& command2)
    {
        const auto materialIndex1 = command1.GetMaterialIndex();
        const auto materialIndex2 = command2.GetMaterialIndex();
        if (materialIndex1 < materialIndex2)
        {
            return true;
        }
        if (materialIndex1 > materialIndex2)
            return false;
        const auto piplineIndex1 = materials[materialIndex1].GetPipelineIndex();
        const auto piplineIndex2 = materials[materialIndex2].GetPipelineIndex();
        return piplineIndex1 < piplineIndex2;
    });
}
} // namespace novus
