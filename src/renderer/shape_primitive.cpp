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
    mesh.name = "quad";
    GLuint ebo;
    std::array<GLuint, 4> vbo{};
    constexpr glm::vec2 vertices[4] = {
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
        constexpr glm::vec3 edge1 = glm::vec3(vertices[1] - vertices[0], 0);
        constexpr glm::vec3 edge2 = glm::vec3(vertices[2] - vertices[0], 0);
        constexpr glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
        constexpr glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

        constexpr float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[0].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[0].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[0].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    }
    std::fill(tangent.begin() + 1, tangent.end(), tangent[0]);

    constexpr unsigned int indices[6] = {
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

Mesh GenerateEmpty()
{
    Mesh mesh{};
    mesh.name = "empty";
    glCreateVertexArrays(1, &mesh.vao);
    
    return mesh;
}

Mesh GenerateCube()
{
    Mesh cube{};
    cube.name = "cube";
    constexpr std::array indices =
    {
        0,1,2,0,3,1,
        4,5,6,6,7,4,
        8,9,10,9,8,11,
        12,13,14,14,15,12,
        16,17,18,18,19,16,
        20,21,22,21,20,23
    };
    constexpr std::array position =
    {
        //Right face
        glm::vec3(0.5f, 0.5f, 0.5f) , //0
        glm::vec3(0.5f, -0.5f, -0.5f) , //1
        glm::vec3(0.5f, 0.5f, -0.5f) , //2
        glm::vec3(0.5f, -0.5f, 0.5f) ,//3
        //Left face                 
        glm::vec3(-0.5f, 0.5f, 0.5f) , //4
        glm::vec3(-0.5f, 0.5f, -0.5f) , //5
        glm::vec3(-0.5f, -0.5f, -0.5f) , //6
        glm::vec3(-0.5f, -0.5f, 0.5f) ,//7
        //Top face 
        glm::vec3(-0.5f, 0.5f, -0.5f) ,//8
        glm::vec3(0.5f, 0.5f, 0.5f) ,//9
        glm::vec3(0.5f, 0.5f, -0.5f) ,//10
        glm::vec3(-0.5f, 0.5f, 0.5f) ,//11
        //Bottom face
        glm::vec3(-0.5f, -0.5f, -0.5f) ,//12
        glm::vec3(0.5f, -0.5f, -0.5f) ,//13
        glm::vec3(0.5f, -0.5f, 0.5f) ,//14
        glm::vec3(-0.5f, -0.5f, 0.5f) ,//15
        //Front face
        glm::vec3(-0.5f, -0.5f, 0.5f) ,//16
        glm::vec3(0.5f, -0.5f, 0.5f) ,//17
        glm::vec3(0.5f, 0.5f, 0.5f) ,//18
        glm::vec3(-0.5f, 0.5f, 0.5f) ,//19
        //Back face
        glm::vec3(-0.5f, -0.5f, -0.5f) ,//20
        glm::vec3(0.5f, 0.5f, -0.5f) ,//21
        glm::vec3(0.5f, -0.5f, -0.5f) ,//22
        glm::vec3(-0.5f, 0.5f, -0.5f) ,//23
    };
    constexpr std::array texCoords = {
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
    };

    constexpr std::array normals =
    {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),

            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),

            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),

            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),

            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),

            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
    };
    

    glm::vec3 tangent[24]{};
    for (int i = 0; i < 36; i += 3)
    {
        std::array triIndices =
        {
            indices[i],
            indices[i+1],
            indices[i+2],
        };
        const glm::vec3 edge1 = position[triIndices[1]] - position[triIndices[0]];
        const glm::vec3 edge2 = position[triIndices[2]] - position[triIndices[0]];
        const glm::vec2 deltaUV1 = texCoords[triIndices[1]] - texCoords[triIndices[0]];
        const glm::vec2 deltaUV2 = texCoords[triIndices[2]] - texCoords[triIndices[0]];

        const float f =
            1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[triIndices[0]].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[triIndices[0]].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[triIndices[0]].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent[triIndices[1]] = tangent[triIndices[0]];
        tangent[triIndices[2]] = tangent[triIndices[0]];
    }
    
    std::array<GLuint, 4> vbo{};
    GLuint ebo;
    glGenBuffers(4, &vbo[0]);
    glGenBuffers(1, &ebo);
    glCheckError();
    glGenVertexArrays(1, &cube.vao);
    glBindVertexArray(cube.vao);
    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, position.size()*sizeof(glm::vec3), position.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size()*sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    // normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(2);
    
    //tangent attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), tangent, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(3);
    
    //ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(int), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
    glCheckError();

    return cube;
}
}
