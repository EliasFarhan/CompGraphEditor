#include "gl/buffer.h"
#include "renderer/model.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "gl/debug.h"

namespace gl
{
void VertexBuffer::CreateFromMesh(const core::refactor::Mesh& mesh)
{
    //Initialize the EBO program
    glGenBuffers(5, vbo.data());
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);
    // 1. bind Vertex Array Object
    glBindVertexArray(vao);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(glm::vec3), mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), static_cast<void*>(0));
    glEnableVertexAttribArray(0);
    //bind texture coords data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh.texCoords.size() * sizeof(glm::vec2), mesh.texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(1);
    // bind normals data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(glm::vec3), mesh.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), static_cast<void*>(0));
    glEnableVertexAttribArray(2);
    // bind tangent data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, mesh.tangents.size(), mesh.tangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), static_cast<void*>(0));
    glEnableVertexAttribArray(3);
    // bind bitangent data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glBufferData(GL_ARRAY_BUFFER, mesh.bitangents.size(), mesh.bitangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), static_cast<void*>(0));
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
