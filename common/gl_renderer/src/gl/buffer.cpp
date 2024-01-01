#include "gl/buffer.h"
#include "renderer/model.h"

#include "gl/debug.h"
#include "utils/log.h"

#include <fmt/format.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#ifdef TRACY_ENABLE
#include <tracy/TracyOpenGL.hpp>
#endif

namespace gl
{
VertexInputBuffer::~VertexInputBuffer()
{
    if(vao != 0)
    {
        LogWarning("Vertex Buffer not deallocated");
    }
}

void VertexInputBuffer::CreateFromMesh(const core::Mesh& mesh)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadBuffer, "Create Vertex Buffer", true);
#endif
    //Initialize the EBO program
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    // 1. bind Vertex Array Object
    glBindVertexArray(vao);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(core::Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(core::Vertex), (void*)offsetof(core::Vertex, position));
    glEnableVertexAttribArray(0);
    //bind texture coords data
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(core::Vertex), (void*)offsetof(core::Vertex, texCoords));
    glEnableVertexAttribArray(1);
    // bind normals data
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(core::Vertex), (void*)offsetof(core::Vertex, normal));
    glEnableVertexAttribArray(2);
    // bind tangent data
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(core::Vertex), (void*)offsetof(core::Vertex, tangent));
    glEnableVertexAttribArray(3);
    // bind bitangent data
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(core::Vertex), (void*)offsetof(core::Vertex, bitangent));
    glEnableVertexAttribArray(4);
    //bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size()*sizeof(unsigned), mesh.indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
    glCheckError();
}

void VertexInputBuffer::Bind()
{
    glBindVertexArray(vao);
}

void VertexInputBuffer::Destroy()
{
    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
        glDeleteBuffers(1, &vbo);
        vbo = 0;
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
}

void BufferManager::Clear()
{
    for (auto& buffer : buffers_)
    {
        if (buffer.ssbo != 0)
        {
            glDeleteBuffers(1, &buffer.ssbo);
        }
    }
    buffers_.clear();
}

core::BufferId BufferManager::CreateBuffer(std::string_view name, std::size_t count, std::size_t size)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadBuffer, "Create SSBO Buffer", true);
#endif
    const auto index = buffers_.size();
    Buffer buffer{};
    glGenBuffers(1, &buffer.ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, count*size, nullptr, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    buffer.data.resize(count*size);
    buffer.name = name;
    buffer.typeSize = size;

    buffers_.emplace_back(buffer);

    return {index};
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
    auto& bufferInfo = buffers_[id.bufferId];
    return
    {
        bufferInfo.data.data(),
        bufferInfo.data.size()/ bufferInfo.typeSize,
        bufferInfo.typeSize
    };
}

core::BufferId BufferManager::GetBuffer(std::string_view bufferName)
{
    const auto it = std::ranges::find_if(buffers_, [&bufferName](const auto& buffer)
    {
        return buffer.name == bufferName;
    });
    if(it != buffers_.end())
    {
        return core::BufferId{ static_cast<std::size_t>(std::distance(buffers_.begin(), it) )};
    }
    return {};
}

void BufferManager::CopyData(std::string_view bufferName, void* dataSrc, std::size_t length)
{
    const auto bufferId = GetBuffer(bufferName);
    const auto arrayBuffer = GetArrayBuffer(bufferId);
    if(length > arrayBuffer.count)
    {
        LogError(fmt::format("Copy Data Error: buffer {} has not enough allocated size. Copy size: {} Buffer size: {}", bufferName, length, arrayBuffer.count));
    }
    std::memcpy(arrayBuffer.data, dataSrc, length);
}

void BufferManager::BindBuffer(core::BufferId id, int bindPoint)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadBuffer, "Copy Buffer to GPU SSBO", true);
#endif
    const auto& buffer = buffers_[id.bufferId];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer.data.size(), buffer.data.data()); //to update partially
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, buffer.ssbo);
}
} // namespace gl

