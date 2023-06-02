#pragma once

#include "renderer/buffer.h"

#include <GL/glew.h>
#include <array>
namespace gl
{
    
class VertexBuffer final : core::VertexBuffer
{
public:
    void CreateFromMesh(const core::Mesh& mesh) override;
    void Bind() override;
    void Destroy() override;

private:
    GLuint vao{};
    GLuint vbo{};
    GLuint ebo{};
};

struct Buffer
{
    GLuint ssbo;
    std::vector<std::uint8_t> data;
};
class BufferManager final : public core::BufferManager
{
public:
    void Clear() override;

    core::BufferId CreateBuffer(std::size_t count, std::size_t size) override;

    void* GetArrayBuffer(core::BufferId id) override;

private:
    std::vector<Buffer> buffers_;
};
} // namespace gl
