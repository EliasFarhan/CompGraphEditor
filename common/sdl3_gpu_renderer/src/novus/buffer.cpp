//
// Created by unite on 10.10.2025.
//
#include "novus/buffer.h"

#include "novus/engine.h"

namespace novus
{
void VertexInputBuffer::CreateFromMesh(const core::Mesh& mesh)
{

    const uint32_t vertexBufferSize = mesh.vertices.size()*sizeof(core::Vertex);
    const uint32_t indexBufferSize = mesh.indices.size()*sizeof(uint32_t);
    TransferBuffer vertexTransferBuffer = GenerateTransferBuffer(vertexBufferSize, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);
    TransferBuffer indexTransferBuffer = GenerateTransferBuffer(indexBufferSize, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);

    //TODO use one command buffer to upload ALL the data instead of a command buffer per mesh
    auto* commandBuffer = SDL_AcquireGPUCommandBuffer(GetDevice());
    auto* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_GPUTransferBufferLocation vertexSrc
    {
        .transfer_buffer = vertexTransferBuffer.get(),
        .offset = 0
    };
    SDL_GPUBufferRegion vertexDest
    {
        .buffer = vertexBuffer_,
        .size = vertexBufferSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &vertexSrc, &vertexDest, false);
    SDL_GPUTransferBufferLocation indexSrc
    {
        .transfer_buffer = indexTransferBuffer.get(),
        .offset = 0
    };
    SDL_GPUBufferRegion indexDest
    {
        .buffer = indexBuffer_,
        .size = indexBufferSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &indexSrc, &indexDest, false);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    //TODO needs to destroy the transfer buffer
}

void VertexInputBuffer::Bind()
{
    //TODO needs to bind to a render pass (given by a set function?)
}

void VertexInputBuffer::Destroy()
{
}

core::BufferId BufferManager::CreateBuffer(std::string_view name, std::size_t count, std::size_t size)
{
}

void BufferManager::Clear()
{
}

core::BufferId BufferManager::GetBuffer(std::string_view bufferName)
{
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
}

void BufferManager::CopyData(std::string_view bufferName, void* dataSrc, std::size_t length)
{
}

void BufferManager::BindBuffer(core::BufferId id, int bindPoint)
{
}
TransferBuffer GenerateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage)
{
    SDL_GPUTransferBufferCreateInfo createInfo{
        .usage = usage,
        .size = size
    };
    auto* transferBuffer = SDL_CreateGPUTransferBuffer(GetDevice(), &createInfo);
    if(transferBuffer == nullptr)
    {
        throw std::runtime_error(std::format("Could no create transfer buffer: {}", SDL_GetError()));
    }
    return TransferBuffer{transferBuffer};
}
} // namespace novus
