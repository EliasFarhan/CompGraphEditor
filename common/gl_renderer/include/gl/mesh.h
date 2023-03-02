#pragma once

#include <GL/glew.h>
#include <string>

#include "renderer/mesh.h"

namespace gl
{
struct Mesh : core::Mesh
{
    [[nodiscard]] std::string_view GetName() const override
    {
        return name;
    }
    std::string name;
    GLuint vao;
};
}
