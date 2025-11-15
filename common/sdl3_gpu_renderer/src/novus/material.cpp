#include "novus/material.h"
#include "engine/scene.h"
#include "novus/buffer.h"
#include <SDL3/SDL_gpu.h>

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
    const renderer::ShaderT& vertexShader,
    const renderer::ShaderT& fragmentShader,
    const BufferManager& bufferManager)
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
}
} // namespace novus
