#pragma once

#include "renderer/buffer.h"

namespace vk
{

class BufferManager final: public core::BufferManager
{
public:
    void Clear() override;
    core::BufferIdx CreateBuffer(std::string_view name, std::size_t count, std::size_t size) override;
    core::BufferIdx GetBuffer(std::string_view bufferName) override;
    core::ArrayBuffer GetArrayBuffer(core::BufferIdx id) override;
    void CopyData(std::string_view bufferName, void* dataSrc, std::size_t length) override;
    void BindBuffer(core::BufferIdx id, int bindPoint) override;
};
}