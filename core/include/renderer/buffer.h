#pragma once

#include "renderer/model.h"

namespace core
{

class VertexInputBuffer
{
public:
    virtual ~VertexInputBuffer() = default;
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
    virtual BufferId CreateBuffer(std::string_view name, std::size_t count, std::size_t size) = 0;
    virtual void Clear() = 0;
    virtual BufferId GetBuffer(std::string_view bufferName) = 0;
    virtual ArrayBuffer GetArrayBuffer(BufferId id) = 0;
    virtual void CopyData(std::string_view bufferName, void* dataSrc, std::size_t length) = 0;
    virtual void BindBuffer(BufferId id, int bindPoint) = 0;
    template<typename T>
    NativeArrayBuffer<T> GetNativeArrayBuffer(BufferId id)
    {
        const auto arrayBuffer = GetArrayBuffer(id);
        return { static_cast<T*>(arrayBuffer.data), arrayBuffer.count / sizeof(T) };
    }

};

} // namespace core
