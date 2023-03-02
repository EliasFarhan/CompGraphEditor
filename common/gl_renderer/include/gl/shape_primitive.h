#pragma once

#include "gl/mesh.h"
#include <glm/vec3.hpp>

namespace gl
{
Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateEmpty();
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset);

}