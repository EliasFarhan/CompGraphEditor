#pragma once

#include <tiny_obj_loader.h>
#include "renderer/mesh.h"

namespace gpr5300
{
struct Model
{
    tinyobj::ObjReader reader;

    void LoadModel(std::string_view path);
    Mesh GenerateMesh(std::string_view meshName) const;
};

}
