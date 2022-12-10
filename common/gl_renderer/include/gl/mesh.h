#pragma once

#include <GL/glew.h>
#include <string>

#include "renderer/mesh.h"

namespace gpr5300::gl
{
struct Mesh : gpr5300::Mesh
{
    std::string_view GetName() const override
    {
        return name;
    }
    std::string name;
    GLuint vao;
};
}
