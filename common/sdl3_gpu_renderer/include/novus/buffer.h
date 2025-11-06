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
class VertexInputBuffer : public core::VertexInputBuffer
{
public:
	void CreateFromMesh(const core::Mesh& mesh) override;

	void Bind() override;

	void Destroy() override;
private:
    SDL_GPUBuffer* vertexBuffer_ = nullptr;
    SDL_GPUBuffer* indexBuffer_ = nullptr;
};

class BufferManager: public core::BufferManager
{
public:
	core::BufferId CreateBuffer(std::string_view name, std::size_t count, std::size_t size) override;

	void Clear() override;

	core::BufferId GetBuffer(std::string_view bufferName) override;

	core::ArrayBuffer GetArrayBuffer(core::BufferId id) override;

	void CopyData(std::string_view bufferName, void* dataSrc, std::size_t length) override;

	void BindBuffer(core::BufferId id, int bindPoint) override;
};
class TransferBuffer
{
public:
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
    void UploadBuffer(void* buffer, size_t length, bool cycle = false, size_t offset = 0)
    {
        auto transferBuffer = get();
        auto* dst = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(GetDevice(), transferBuffer, cycle));
        std::memcpy(dst+offset, buffer, length);
        SDL_UnmapGPUTransferBuffer(novus::GetDevice(), transferBuffer);
    }
    SDL_GPUTransferBuffer* get();
private:
    SDL_GPUTransferBuffer* transferBuffer_ = nullptr;
};
TransferBuffer GenerateTransferBuffer(uint32_t size, SDL_GPUTransferBufferUsage usage);
}

#endif //NEKO2_BUFFER_H
