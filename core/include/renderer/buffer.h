#pragma once

#include <glm/mat2x2.hpp>
#include <glm/vec4.hpp>


#include "generated/internal_renderer_generated.h"
#include "renderer/model.h"

namespace core
{

class VertexInputBuffer
{
public:
    virtual ~VertexInputBuffer() = default;
    virtual void CreateFromMesh(const Mesh& mesh, void* uploadData=nullptr) = 0;
    virtual void Bind(void* renderData = nullptr) = 0;
    virtual void Destroy() = 0;
};

/**
 * BufferManager is a class that defines a system that manages dynamic GPU buffer
 * (SSBO in OpenGL and Storage Buffer for Vulkan)
 */
struct BufferIdx
{
    std::size_t bufferId = static_cast<std::size_t>(-1);
};

struct ArrayBuffer
{
    void* data = nullptr;
    std::size_t count = static_cast<std::size_t>(-1);
    std::size_t typeSize = static_cast<std::size_t>(-1);
};

template<typename T>
struct NativeArrayBuffer
{
    T* data = nullptr;
    std::size_t length = 0;
};

class BufferManager
{
public:
    virtual ~BufferManager() = default;
    virtual BufferIdx CreateBuffer(std::string_view name, std::size_t count, std::size_t size) = 0;
    virtual void Clear() = 0;
    virtual BufferIdx GetBuffer(std::string_view bufferName) const = 0;
    virtual void CopyData(BufferIdx index, const void* dataSrc, std::size_t length) = 0;
};
} // namespace core
