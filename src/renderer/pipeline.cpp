
#include <fmt/format.h>

#include "renderer/pipeline.h"

#include "engine/filesystem.h"
#include "utils/log.h"

namespace gpr5300
{
void Shader::LoadShader(std::string_view path, ShaderType shaderType)
{
    shaderType_ = shaderType;
    const auto& filesystem = FilesystemLocator::get();
    GLenum glType = 0;
    switch (shaderType)
    {
    case ShaderType::VERTEX:
        glType = GL_VERTEX_SHADER;
        break;
    case ShaderType::FRAGMENT:
        glType = GL_FRAGMENT_SHADER;
        break;
    case ShaderType::COMPUTE:
        glType = GL_COMPUTE_SHADER;
        break;
    default:
        return;
    }
    if(filesystem.IsRegularFile(path))
    {
        const auto file = filesystem.LoadFile(path);
        const GLuint shader = glCreateShader(glType);


        glShaderSource(shader, 1, reinterpret_cast<const GLchar* const*>( & file.data), nullptr);
        glCompileShader(shader);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            constexpr GLsizei infoLogSize = 512;
            char infoLog[infoLogSize];
            glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog);
            Error(fmt::format("Shader compilation failed with this log:\n{}\nShader Path:\n{}",
                infoLog, path));
        }
        name_ = shader;
    }
}
}
