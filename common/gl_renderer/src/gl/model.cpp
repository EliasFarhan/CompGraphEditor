#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>
#undef TINYOBJLOADER_IMPLEMENTATION

#include "engine/filesystem.h"
#include "gl/model.h"
#include "utils/log.h"
#include "gl/debug.h"

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GL/glew.h>
#include <fmt/format.h>
#include <array>
#include <unordered_set>
#include <ranges>

namespace gpr5300::gl
{
void Model::LoadModel(std::string_view path)
{
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.vertex_color = false;
    const auto& filesystem = FilesystemLocator::get();
    const auto modelFile = filesystem.LoadFile(path);
    if(!reader.ParseFromString(reinterpret_cast<const char*>(modelFile.data), "", config))
    {
        LogError(fmt::format("Could not correctly load obj: {}\n{}", path, reader.Error()));
    }

}

Mesh Model::GenerateMesh(std::string_view meshName) const
{
    Mesh mesh{};
    mesh.name = meshName;
    const auto& shapes = reader.GetShapes();
    auto it = std::ranges::find_if(shapes, [meshName](const auto& shape)
        {
            return shape.name == meshName;
        });
    if (it == shapes.end())
    {
        LogError(fmt::format("Could not generate mesh {} from obj", meshName));
        return mesh;
    }
    auto& shape = *it;

    std::array<GLuint, 4> vbo{};
    GLuint ebo;
    glGenBuffers(4, &vbo[0]);
    glGenBuffers(1, &ebo);
    glCheckError();
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    std::unordered_set<int> meshIndicesSet;
    std::vector<tinyobj::index_t> meshIndices;
    std::vector<int> indices;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    const auto& attrib = reader.GetAttrib();

    int actualIndex = 0;

    for (const auto& indice : shape.mesh.indices)
    {
        const auto vertexIndex = indice.vertex_index;
        if (meshIndicesSet.contains(vertexIndex))
        {
            const auto indexIt = std::ranges::find_if(meshIndices, [vertexIndex](const auto index)
            {
                return vertexIndex == index.vertex_index;
            });
            const auto ourIndex = std::distance(meshIndices.begin(), indexIt);

            meshIndices.push_back(indice);
            indices.push_back(indices[ourIndex]);
            continue;
            
        }
        meshIndices.push_back(indice);
        meshIndicesSet.emplace(vertexIndex);
        indices.push_back(actualIndex);
        const auto pos = glm::vec3(
            attrib.vertices[vertexIndex * 3 + 0],
            attrib.vertices[vertexIndex * 3 + 1],
            attrib.vertices[vertexIndex * 3 + 2]);
        positions.push_back(pos);

        const auto texCoordsIndex = indice.texcoord_index;
        const auto texCoord = glm::vec2(
            attrib.texcoords[texCoordsIndex * 2 + 0],
            attrib.texcoords[texCoordsIndex * 2 + 1]
        );
        texCoords.push_back(texCoord);

        const auto normalIndex = indice.normal_index;
        const auto normal = glm::vec3(
            attrib.normals[normalIndex * 3 + 0],
            attrib.normals[normalIndex * 3 + 1],
            attrib.normals[normalIndex * 3 + 2]);
        normals.push_back(normal);
        actualIndex++;
        
    }

    std::vector<glm::vec3> tangents{};
    tangents.resize(normals.size());
    for (int i = 0; i < indices.size(); i += 3)
    {
        std::array triIndices =
        {
            indices[i],
            indices[i + 1],
            indices[i + 2],
        };
        const glm::vec3 edge1 = positions[triIndices[1]] - positions[triIndices[0]];
        const glm::vec3 edge2 = positions[triIndices[2]] - positions[triIndices[0]];
        const glm::vec2 deltaUV1 = texCoords[triIndices[1]] - texCoords[triIndices[0]];
        const glm::vec2 deltaUV2 = texCoords[triIndices[2]] - texCoords[triIndices[0]];

        const float f =
            1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangents[triIndices[0]].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangents[triIndices[0]].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangents[triIndices[0]].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangents[triIndices[1]] = tangents[triIndices[0]];
        tangents[triIndices[2]] = tangents[triIndices[0]];
    }
    
    //ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);


    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    // normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(2);

    // tangent attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), tangents.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(3);
    glCheckError();

    return mesh;
}
} // namespace gpr5300
