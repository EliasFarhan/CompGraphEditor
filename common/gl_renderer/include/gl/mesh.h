#pragma once

#include <GL/glew.h>
#include <string>

namespace gpr5300::gl
{
struct Mesh
{
    std::string name;
    GLuint vao;
};
}