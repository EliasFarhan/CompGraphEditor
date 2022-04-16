#include "renderer/shape_primitive.h"
#include "renderer/debug.h"

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace gpr5300
{

Mesh GenerateQuad()
{
    Mesh mesh{};
    GLuint ebo;
    std::array<GLuint, 4> vbo{};
    const glm::vec2 vertices[4] = {
        glm::vec2(0.5f, 0.5f),  // top right
        glm::vec2(0.5f, -0.5f),  // bottom right
        glm::vec2(-0.5f, -0.5f),  // bottom left
        glm::vec2(-0.5f, 0.5f)  // top left
    };

    constexpr glm::vec2 texCoords[4] = {
        glm::vec2(1.0f, 1.0f),      // top right
        glm::vec2(1.0f, 0.0f),   // bottom right
        glm::vec2(0.0f, 0.0f),   // bottom left
        glm::vec2(0.0f, 1.0f),   // bottom left
    };

    constexpr glm::vec3 normals[4] = {
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1),
        glm::vec3(0,0,-1)
    };

    std::array<glm::vec3, 4> tangent{};

    {
        const glm::vec3 edge1 = glm::vec3(vertices[1] - vertices[0], 0);
        const glm::vec3 edge2 = glm::vec3(vertices[2] - vertices[0], 0);
        const glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
        const glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[0].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[0].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[0].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    }
    std::fill(tangent.begin() + 1, tangent.end(), tangent[0]);

    unsigned int indices[6] = {
        // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    //Initialize the EBO program
    glGenBuffers(4, &vbo[0]);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &mesh.vao);
    // 1. bind Vertex Array Object
    glBindVertexArray(mesh.vao);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    //bind texture coords data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    // bind normals data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);
    // bind tangent data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), &tangent[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    //bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glCheckError();

    return mesh;
}
}