#pragma once

#include "renderer/buffer.h"

#include <GL/glew.h>
#include <array>
namespace gl
{
    
class VertexInputBuffer final : core::VertexInputBuffer
{
public:
    ~VertexInputBuffer() override;
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
    std::string name;
    std::vector<std::uint8_t> data;
    std::uint32_t typeSize = 0;
    GLuint ssbo;
};
class BufferManager final : public core::BufferManager
{
public:
    void Clear() override;

    core::BufferId CreateBuffer(std::string_view name, std::size_t count, std::size_t size) override;

    core::ArrayBuffer GetArrayBuffer(core::BufferId id) override;
    core::BufferId GetBuffer(std::string_view bufferName) override;
    void CopyData(std::string_view bufferName, void* dataSrc, std::size_t length) override;
    void BindBuffer(core::BufferId id, int bindPoint) override;
private:
    std::vector<Buffer> buffers_;
};
} // namespace gl
