#include "gl/buffer.h"
#include "renderer/model.h"

#include "gl/debug.h"
#include "utils/log.h"

#include <fmt/format.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace gl
{

void VertexBuffer::CreateFromMesh(const core::Mesh& mesh)
{
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

void VertexBuffer::Bind()
{
    glBindVertexArray(vao);
}

void VertexBuffer::Destroy()
{
    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
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
    const auto index = buffers_.size();
    Buffer buffer{};
    glGenBuffers(1, &buffer.ssbo);
    buffer.data.resize(count*size);
    buffer.name = name;
    buffer.typeSize = size;

    buffers_.emplace_back(buffer);

    return {index};
}

core::ArrayBuffer BufferManager::GetArrayBuffer(core::BufferId id)
{
    return
    {
        buffers_[id.bufferId].data.data(),
        buffers_[id.bufferId].data.size()/buffers_[id.bufferId].typeSize
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
    if(length > arrayBuffer.length)
    {
        LogError(fmt::format("Copy Data Error: buffer {} has not enough allocated size. Copy size: {} Buffer size: {}", bufferName, length, arrayBuffer.length));
    }
    std::memcpy(arrayBuffer.data, dataSrc, length);
}
} // namespace gl

