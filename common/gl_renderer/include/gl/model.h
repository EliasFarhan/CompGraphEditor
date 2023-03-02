#pragma once

#include <tiny_obj_loader.h>
#include "gl/mesh.h"

namespace gl
{
struct Model
{
    tinyobj::ObjReader reader;

    void LoadModel(std::string_view path);
    [[nodiscard]] Mesh GenerateMesh(std::string_view meshName) const;
};

}
