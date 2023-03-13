#pragma once

#include "renderer/buffer.h"

#include <GL/glew.h>
#include <array>

namespace gl
{
    
class VertexBuffer final : core::VertexBuffer
{
public:
    void CreateFromMesh(const core::refactor::Mesh& mesh) override;
    void Bind() override;
    void Destroy() override;

private:
    GLuint vao{};
    std::array<GLuint, 5> vbo{};
    GLuint ebo{};
};

} // namespace gl
