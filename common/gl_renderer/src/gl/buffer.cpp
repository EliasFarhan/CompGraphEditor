#include "gl/buffer.h"
#include "renderer/model.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "gl/debug.h"

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

} // namespace gl
