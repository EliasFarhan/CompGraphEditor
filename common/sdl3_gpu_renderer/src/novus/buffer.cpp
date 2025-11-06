#include "novus/buffer.h"

#include "novus/engine.h"

namespace novus
{
void VertexInputBuffer::CreateFromMesh(const core::Mesh& mesh, void* uploadData)
{
    SDL_GPUCopyPass* copyPass = static_cast<SDL_GPUCopyPass*>(uploadData);
    const uint32_t vertexBufferSize = mesh.vertices.size()*sizeof(core::Vertex);
    const uint32_t indexBufferSize = mesh.indices.size()*sizeof(uint32_t);

    SDL_GPUBufferCreateInfo vertexBufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = vertexBufferSize
    };
    vertexBuffer_ = SDL_CreateGPUBuffer(GetDevice(), &vertexBufferCreateInfo);
    SDL_GPUBufferCreateInfo indexBufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_INDEX,
        .size = indexBufferSize
    };
    indexBuffer_ = SDL_CreateGPUBuffer(GetDevice(), &indexBufferCreateInfo);

    vertexTransferBuffer_ = GenerateTransferBuffer(vertexBufferSize, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);
    indexTransferBuffer_ = GenerateTransferBuffer(indexBufferSize, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);

    vertexTransferBuffer_.UploadRange(mesh.vertices);
    indexTransferBuffer_.UploadRange(mesh.indices);

    SDL_GPUTransferBufferLocation vertexSrc
    {
        .transfer_buffer = vertexTransferBuffer_.get(),
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
        .transfer_buffer = indexTransferBuffer_.get(),
        .offset = 0
    };
    SDL_GPUBufferRegion indexDest
    {
        .buffer = indexBuffer_,
        .size = indexBufferSize,
    };
    SDL_UploadToGPUBuffer(copyPass, &indexSrc, &indexDest, false);

}

void VertexInputBuffer::Bind(void* renderData)
{
    if (vertexBuffer_ == nullptr || indexBuffer_ == nullptr)
        return;
    SDL_GPURenderPass* renderPass = static_cast<SDL_GPURenderPass*>(renderData);
    SDL_GPUBufferBinding bufferBinding{
        .buffer = vertexBuffer_,
        .offset = 0
    };
    SDL_BindGPUVertexBuffers(renderPass, 0, &bufferBinding, 1);
    SDL_GPUBufferBinding indexBufferBinding{
        .buffer = indexBuffer_,
        .offset = 0
    };
    SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

}
void VertexInputBuffer::ClearTransferBuffers()
{
    vertexTransferBuffer_.Destroy();
    indexTransferBuffer_.Destroy();
}

void VertexInputBuffer::Destroy()
{
    ClearTransferBuffers();
    if (vertexBuffer_ != nullptr)
    {
        SDL_ReleaseGPUBuffer(GetDevice(), vertexBuffer_);
        vertexBuffer_ = nullptr;
    }
    if (indexBuffer_ != nullptr)
    {
        SDL_ReleaseGPUBuffer(GetDevice(), indexBuffer_);
        indexBuffer_ = nullptr;
    }
}

core::BufferId BufferManager::CreateBuffer(std::string_view name, std::size_t count, std::size_t size)
{
    return {};
}

void BufferManager::Clear()
{
}

core::BufferId BufferManager::GetBuffer(std::string_view bufferName)
{
    return {};
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
    return {};
}

void BufferManager::CopyData(std::string_view bufferName, void* dataSrc, std::size_t length)
{
}

void BufferManager::BindBuffer(core::BufferId id, int bindPoint)
{
}
void TransferBuffer::Destroy()
{
    if (transferBuffer_ == nullptr)
        return;
    SDL_ReleaseGPUTransferBuffer(GetDevice(), transferBuffer_);
    transferBuffer_ = nullptr;
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
