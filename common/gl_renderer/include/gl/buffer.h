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
} // namespace gl
