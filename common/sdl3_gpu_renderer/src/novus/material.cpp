#include "novus/material.h"
#include "engine/scene.h"
#include "novus/buffer.h"
#include <SDL3/SDL_gpu.h>

#include "novus/framebuffer.h"
#include "novus/texture.h"

namespace novus
{
Material::Material(Pipeline* pipeline) : pipeline_(pipeline)
{

}
std::string_view Material::GetName() const
{
    return name_;
}
void Material::Load(const renderer::MaterialT& materialInfo, 
    const BufferManager& bufferManager, 
    std::span<core::TextureId> textures,
    std::span<Framebuffer> framebuffers)
{
    name_ = materialInfo.name;
    pipelineIndex_ = materialInfo.pipeline_index;

    int vertexBindingCount = 0;
    int fragmentBindingCount = 0;
    for (const auto& storageBufferBinding: materialInfo.storage_buffer_bindings)
    {
        switch (storageBufferBinding.shader_stage)
        {
        case internal::ShaderStage_VERTEX:
            vertexBindingCount++;
            break;
        case internal::ShaderStage_FRAGMENT:
            fragmentBindingCount++;
            break;
        default:
            break;
        }
    }
    vertexStorageBuffers.resize(vertexBindingCount);
    fragmentStorageBuffers.resize(fragmentBindingCount);
    for (const auto& storageBufferBinding: materialInfo.storage_buffer_bindings)
    {
        const auto bufferIdx = bufferManager.GetBuffer(storageBufferBinding.buffer_name);
        const auto& buffer = bufferManager.GetStorageBuffer(bufferIdx);
        switch (storageBufferBinding.shader_stage)
        {
        case internal::ShaderStage_VERTEX:
            vertexStorageBuffers[storageBufferBinding.binding] = buffer.gpuBuffer;
            break;
        case internal::ShaderStage_FRAGMENT:
            fragmentStorageBuffers[storageBufferBinding.binding] = buffer.gpuBuffer;
            break;
        default:
            break;
        }
    }
    const auto vertexSamplerCount = std::ranges::count_if(materialInfo.texture_bindings, [](const auto& textureBindingInfo)
    {
        return textureBindingInfo.set == 0;
    });
    vertexTextures.resize(vertexSamplerCount);
    const auto fragmentSamplerCount = std::ranges::count_if(materialInfo.texture_bindings, [](const auto& textureBindingInfo)
    {
        return textureBindingInfo.set == 2;
    });
    fragmentTextures.resize(fragmentSamplerCount);
    auto& textureManager = GetTextureManager();
    for (auto& textureBinding: materialInfo.texture_bindings)
    {
        if (textureBinding.framebuffer_index == -1)
        {
            switch (textureBinding.set)
            {
            case 0:
            {
                auto& texture = textureManager.GetTexture(textures[textureBinding.texture_index]);
                vertexTextures[textureBinding.binding] = {.texture = texture.texture, .sampler = texture.sampler};
                break;
            }
            case 2:
            {
                auto& texture = textureManager.GetTexture(textures[textureBinding.texture_index]);
                fragmentTextures[textureBinding.binding] = {.texture = texture.texture, .sampler = texture.sampler};
                break;
            }
            default:
                break;
            }
        }
        else
        {switch (textureBinding.set)
        {
        case 0:
        {
            auto& texture = framebuffers[textureBinding.framebuffer_index].GetColorTextures()[textureBinding.texture_index];
            vertexTextures[textureBinding.binding] = {.texture = texture.texture, .sampler = texture.sampler};
            break;
        }
        case 2:
        {
            auto& texture = framebuffers[textureBinding.framebuffer_index].GetColorTextures()[textureBinding.texture_index];
            fragmentTextures[textureBinding.binding] = {.texture = texture.texture, .sampler = texture.sampler};
            break;
        }
        default:
            break;
        }

        }
    }
}
void Material::Bind(void* renderData)
{
    pipeline_->Bind(renderData);
    auto* renderPass = static_cast<SDL_GPURenderPass*>(renderData);
    if (!vertexStorageBuffers.empty())
    {
        SDL_BindGPUVertexStorageBuffers(renderPass, 0, vertexStorageBuffers.data(), vertexStorageBuffers.size());
    }
    if (!fragmentStorageBuffers.empty())
    {
        SDL_BindGPUFragmentStorageBuffers(renderPass, 0, fragmentStorageBuffers.data(), fragmentStorageBuffers.size());
    }
    if (!vertexTextures.empty())
    {
        SDL_BindGPUVertexSamplers(renderPass, 0, vertexTextures.data(), vertexTextures.size());
    }
    if (!fragmentTextures.empty())
    {
        SDL_BindGPUFragmentSamplers(renderPass, 0, fragmentTextures.data(), fragmentTextures.size());
    }

}
} // namespace novus
