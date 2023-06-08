#include "renderer/mesh.h"

namespace core
{
    

Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Mesh mesh{};
    mesh.name = "quad";
    mesh.vertices = {
        {glm::vec3(0.5f, 0.5f, 0.0f) * scale + offset, glm::vec2(1.0f, 1.0f), glm::vec3(0,0,-1)},// top right
        {glm::vec3(0.5f, -0.5f, 0.0f) * scale + offset, glm::vec2(1.0f, 0.0f), glm::vec3(0,0,-1)}, // bottom right
        {glm::vec3(-0.5f, -0.5f, 0.0f) * scale + offset, glm::vec2(0.0f, 0.0f), glm::vec3(0,0,-1)},  // bottom left
        {glm::vec3(-0.5f, 0.5f, 0.0f) * scale + offset, glm::vec2(0.0f, 1.0f), glm::vec3(0,0,-1)},// top left
    };
    
    {
        const glm::vec3 edge1 = mesh.vertices[1].position - mesh.vertices[0].position;
        const glm::vec3 edge2 = mesh.vertices[2].position - mesh.vertices[0].position;
        const glm::vec2 deltaUV1 = mesh.vertices[1].texCoords - mesh.vertices[0].texCoords;
        const glm::vec2 deltaUV2 = mesh.vertices[2].texCoords - mesh.vertices[0].texCoords;

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        mesh.vertices[0].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        mesh.vertices[0].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        mesh.vertices[0].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        mesh.vertices[0].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        mesh.vertices[0].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        mesh.vertices[0].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        mesh.vertices[1].tangent = mesh.vertices[0].tangent;
        mesh.vertices[2].tangent = mesh.vertices[0].tangent;

        mesh.vertices[1].bitangent = mesh.vertices[0].bitangent;
        mesh.vertices[2].bitangent = mesh.vertices[0].bitangent;
    }

    mesh.indices = {
        // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    return mesh;
}
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    Mesh cube{};
    cube.name = "cube";
    cube.indices =
    {
        0,1,2,0,3,1,
        4,5,6,6,7,4,
        8,9,10,9,8,11,
        12,13,14,14,15,12,
        16,17,18,18,19,16,
        20,21,22,21,20,23
    };
    cube.vertices =
    {
        //Right face
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(1.0f, 0.0f, 0.0f),},//0  
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//1
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//2
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(1.0f, 0.0f, 0.0f), },//3
        //Left face                 
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//4
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//5
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(-1.0f, 0.0f, 0.0f),}, //6
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(-1.0f, 0.0f, 0.0f),},//7
        //Top face 
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//8
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//9
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//10
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 1.0f, 0.0f),},//11
        //Bottom face
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, -1.0f, 0.0f),}, //12
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//13
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//14
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, -1.0f, 0.0f),},//15
        //Front face
        {glm::vec3(-0.5f, -0.5f, 0.5f) * scale + offset ,glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//16
        {glm::vec3(0.5f, -0.5f, 0.5f) * scale + offset , glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//17
        {glm::vec3(0.5f, 0.5f, 0.5f) * scale + offset ,  glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//18
        {glm::vec3(-0.5f, 0.5f, 0.5f) * scale + offset , glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 0.0f, 1.0f),},//19
        //Back face
        {glm::vec3(-0.5f, -0.5f, -0.5f) * scale + offset ,glm::vec2(0.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//20
        {glm::vec3(0.5f, 0.5f, -0.5f) * scale + offset ,  glm::vec2(1.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//21
        {glm::vec3(0.5f, -0.5f, -0.5f) * scale + offset , glm::vec2(1.0f, 0.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//22
        {glm::vec3(-0.5f, 0.5f, -0.5f) * scale + offset , glm::vec2(0.0f, 1.0f),glm::vec3(0.0f, 0.0f, -1.0f),},//23
    };
    for (int i = 0; i < 36; i += 3)
    {
        std::array triIndices =
        {
            cube.indices[i],
            cube.indices[i + 1],
            cube.indices[i + 2],
        };
        const glm::vec3 edge1 = cube.vertices[triIndices[1]].position - cube.vertices[triIndices[0]].position;
        const glm::vec3 edge2 = cube.vertices[triIndices[2]].position - cube.vertices[triIndices[0]].position;
        const glm::vec2 deltaUV1 = cube.vertices[triIndices[1]].texCoords - cube.vertices[triIndices[0]].texCoords;
        const glm::vec2 deltaUV2 = cube.vertices[triIndices[2]].texCoords - cube.vertices[triIndices[0]].texCoords;

        const float f =
            1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        cube.vertices[triIndices[0]].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        cube.vertices[triIndices[0]].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        cube.vertices[triIndices[0]].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        cube.vertices[triIndices[1]].tangent = cube.vertices[triIndices[0]].tangent;
        cube.vertices[triIndices[2]].tangent = cube.vertices[triIndices[0]].tangent;

        cube.vertices[triIndices[0]].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        cube.vertices[triIndices[0]].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        cube.vertices[triIndices[0]].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        cube.vertices[triIndices[1]].bitangent = cube.vertices[triIndices[0]].bitangent;
        cube.vertices[triIndices[2]].bitangent = cube.vertices[triIndices[0]].bitangent;
    }
    return cube;
}
Mesh GenerateSphere(float scale, glm::vec3 offset)
{
    constexpr std::size_t segment = sphereSegments;
    Mesh mesh{};
    mesh.name = "sphere";
    
    mesh.vertices.reserve((segment + 1) * (segment + 1));
    mesh.indices.reserve(2*mesh.vertices.capacity());
    for (unsigned int y = 0; y <= segment; ++y)
    {
        for (unsigned int x = 0; x <= segment; ++x)
        {
            float xSegment = static_cast<float>(x) / static_cast<float>(segment);
            float ySegment = static_cast<float>(y) / static_cast<float>(segment);
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            mesh.vertices.push_back(
                {
                    { xPos, yPos, zPos },
                    { xSegment, ySegment },
                    {xPos, yPos, zPos}});
            
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < segment; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= segment; ++x)
            {
                mesh.indices.push_back((y + 1) * (segment + 1) + x);
                mesh.indices.push_back(y * (segment + 1) + x);
            }
        }
        else
        {
            for (int x = segment; x >= 0; --x)
            {
                mesh.indices.push_back(y * (segment + 1) + x);
                mesh.indices.push_back((y + 1) * (segment + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    std::size_t indexCount_ = mesh.indices.size();
    for (size_t i = 0; i < indexCount_ - 2; i++)
    {
        const glm::vec3 edge1 = mesh.vertices[mesh.indices[i + 1]].position - mesh.vertices[mesh.indices[i]].position;
        const glm::vec3 edge2 = mesh.vertices[mesh.indices[i + 2]].position - mesh.vertices[mesh.indices[i]].position;
        const glm::vec2 deltaUV1 = mesh.vertices[mesh.indices[i + 1]].texCoords - mesh.vertices[mesh.indices[i]].texCoords;
        const glm::vec2 deltaUV2 = mesh.vertices[mesh.indices[i + 2]].texCoords - mesh.vertices[mesh.indices[i]].texCoords;

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        mesh.vertices[mesh.indices[i]].tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        mesh.vertices[mesh.indices[i]].tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        mesh.vertices[mesh.indices[i]].tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        mesh.vertices[mesh.indices[i]].bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        mesh.vertices[mesh.indices[i]].bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        mesh.vertices[mesh.indices[i]].bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    }
    return mesh;
}

} // namespace core
