//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_BUFFER_H
#define NEKO2_BUFFER_H
#include <SDL3/SDL_gpu.h>


#include "engine.h"
#include "renderer/buffer.h"

namespace novus
{

class TransferBuffer
{
public:
    TransferBuffer() = default;
    explicit TransferBuffer(SDL_GPUTransferBuffer* buffer): transferBuffer_(buffer) {}
    template<typename T>
    void Upload(const T& src, bool cycle = false, size_t offset = 0)
    {
        auto transferBuffer = get();
        auto* dst = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(GetDevice(), transferBuffer, cycle));
        std::memcpy(dst+offset, &src, sizeof(T));
        SDL_UnmapGPUTransferBuffer(novus::GetDevice(), transferBuffer);
    }
    template<std::ranges::contiguous_range Range>
    void UploadRange(const Range& range, bool cycle = false, size_t offset = 0)
    {
        auto transferBuffer = get();
        auto* dst = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(GetDevice(), transferBuffer, cycle));
        std::memcpy(dst+offset, std::ranges::data(range), std::ranges::size(range)*sizeof(std::ranges::range_value_t<Range>));
        SDL_UnmapGPUTransferBuffer(novus::GetDevice(), transferBuffer);
    }
    void UploadBuffer(const void* buffer, size_t length, bool cycle = false, size_t offset = 0)
    {
        auto transferBuffer = get();
        auto* dst = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(GetDevice(), transferBuffer, cycle));
        std::memcpy(dst+offset, buffer, length);
        SDL_UnmapGPUTransferBuffer(novus::GetDevice(), transferBuffer);
    }
    void Destroy();
    SDL_GPUTransferBuffer* get(){return transferBuffer_;}
private:
    SDL_GPUTransferBuffer* transferBuffer_ = nullptr;
};
TransferBuffer GenerateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage);

class VertexInputBuffer : public core::VertexInputBuffer
{
public:
	void CreateFromMesh(const core::Mesh& mesh, void* uploadData=nullptr) override;

	void Bind(void* renderData = nullptr) override;

    void ClearTransferBuffers();
	void Destroy() override;
private:
    SDL_GPUBuffer* vertexBuffer_ = nullptr;
    SDL_GPUBuffer* indexBuffer_ = nullptr;
    TransferBuffer vertexTransferBuffer_;
    TransferBuffer indexTransferBuffer_;
};

struct StorageBuffer
{
    TransferBuffer transferBuffer;
    SDL_GPUBuffer* gpuBuffer;
    uint32_t size;
    bool isDirty = false;
};

class BufferManager: public core::BufferManager
{
public:
    BufferManager() = default;
    BufferManager(const BufferManager&) = delete;
    BufferManager& operator=(const BufferManager&) = delete;
    BufferManager(BufferManager&& other) noexcept:
        storageBufferMap_(std::move(other.storageBufferMap_)), storageBuffers_(std::move(other.storageBuffers_))
    {
        
    }
    BufferManager& operator=(BufferManager&& other) noexcept
    {
        std::swap(storageBuffers_, other.storageBuffers_);
        std::swap(storageBufferMap_, other.storageBufferMap_);
        return *this;
    }
	core::BufferIdx CreateBuffer(std::string_view name, std::size_t count, std::size_t size) override;

	void Clear() override;

	core::BufferIdx GetBuffer(std::string_view bufferName) const override;


	void CopyData(core::BufferIdx index, const void* dataSrc, std::size_t length) override;


    void UploadStorageBuffers(SDL_GPUCommandBuffer* commandBuffer);

    const StorageBuffer& GetStorageBuffer(core::BufferIdx id) const;
private:
    std::unordered_map<std::string, core::BufferIdx> storageBufferMap_;
    std::vector<StorageBuffer> storageBuffers_;
};

}

#endif //NEKO2_BUFFER_H
