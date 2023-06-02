#pragma once

#include "renderer/buffer.h"

namespace vk
{

class BufferManager final: public core::BufferManager
{
public:
    void Clear() override;

    core::BufferId CreateBuffer(std::size_t count, std::size_t size) override;

    void *GetArrayBuffer(core::BufferId id) override;

};
}