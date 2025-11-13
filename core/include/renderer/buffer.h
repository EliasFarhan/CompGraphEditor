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

namespace experimental
{
constexpr size_t GetPrimitveSize(novus::internal::AttributeType type)
{
    switch (type)
    {
    case novus::internal::AttributeType_VEC2:
        return sizeof(glm::vec2);
    case novus::internal::AttributeType_VEC3:
        return sizeof(glm::vec3);
    case novus::internal::AttributeType_VEC4:
        return sizeof(glm::vec4);
    case novus::internal::AttributeType_MAT2:
        return sizeof(glm::mat2);
    case novus::internal::AttributeType_MAT3:
        return sizeof(glm::mat3);
    case novus::internal::AttributeType_MAT4:
        return sizeof(glm::mat4);
    case novus::internal::AttributeType_INT:
        return sizeof(int);
    case novus::internal::AttributeType_IVEC2:
        return sizeof(glm::ivec2);
    case novus::internal::AttributeType_IVEC3:
        return sizeof(glm::ivec3);
    case novus::internal::AttributeType_IVEC4:
        return sizeof(glm::ivec4);
    case novus::internal::AttributeType_BOOL:
        return sizeof(bool);
    default:
        break;
    }
    return 0;
}
struct AttributeBuffer
{
    void* data = nullptr;
    size_t size = 0;
    uint8_t type = novus::internal::AttributeType_VOID;
};
class BufferManager
{
public:
    virtual void AllocateUniformBuffer(std::string_view uniformName);
    virtual void PushUniformData(novus::internal::ShaderStage stage, int32_t slot) = 0;
private:
    std::unordered_map<std::string, AttributeBuffer> bufferAttributeMap_;
};
}

} // namespace core
