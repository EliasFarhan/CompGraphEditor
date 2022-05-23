#include "renderer/debug.h"
#include "utils/log.h"
#include <fmt/format.h>
#include <GL/glew.h>

namespace gpr5300
{

void CheckGlError(const char *file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::string log;
        // Process/log the error.
        switch (err)
        {
        case GL_INVALID_ENUM:
            log = "GL Invalid Enum";
            break;
        case GL_INVALID_VALUE:
            log = "GL Invalid Value";
            break;
        case GL_INVALID_OPERATION:
            log = "GL Invalid Operation";
            break;
        case GL_OUT_OF_MEMORY:
            log = "GL Out Of Memory";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            log = "GL Invalid Framebuffer Operation";
            break;
        default:
            continue;
        }
        LogError(fmt::format("{} in file: {} at line: {}", log, file, line));
    }
}
}
