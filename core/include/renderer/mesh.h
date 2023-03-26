#pragma once


#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace core
{

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

/**
 * @brief Mesh is an interface to the loading of 3d mesh (aka a bunch a triangle) with
 * positions, texcoords, normal
*/
struct Mesh
{
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;
    unsigned materialIndex = std::numeric_limits<unsigned>::max();
};

Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateSphere(float scale, glm::vec3 offset);

} // namespace core
