#pragma once

#include <GL/glew.h>

namespace gpr5300
{
    //A loaded image in memory
    class Image
    {

    };

    //A loaded texture in GPU that can be used in OpenGL
    struct Texture
    {
        GLuint name = 0;
    };
} // namespace gpr5300
