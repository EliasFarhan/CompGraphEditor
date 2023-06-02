#pragma once

#include "renderer/model.h"

namespace core
{

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;
    virtual void CreateFromMesh(const Mesh& mesh) = 0;
    virtual void Bind() = 0;
    virtual void Destroy() = 0;
};

/**
 * BufferManager is a class that defines a system that manages dynamic GPU buffer
 * (SSBO in OpenGL and Storage Buffer for Vulkan)
 */
struct BufferId
{
    std::size_t bufferId = std::numeric_limits<std::size_t>::max();
};

class BufferManager
{
public:
    virtual void Clear() = 0;
    virtual BufferId CreateBuffer(std::size_t count, std::size_t size) = 0;
    virtual void* GetArrayBuffer(BufferId id) = 0;
};

} // namespace core
