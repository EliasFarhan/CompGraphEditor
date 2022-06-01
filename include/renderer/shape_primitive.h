#pragma once

#include "renderer/mesh.h"

namespace gpr5300
{
Mesh GenerateQuad(glm::vec3 scale, glm::vec3 offset);
Mesh GenerateEmpty();
Mesh GenerateCube(glm::vec3 scale, glm::vec3 offset);

}