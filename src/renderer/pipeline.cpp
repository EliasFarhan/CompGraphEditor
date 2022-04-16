
#include <fmt/format.h>

#include "renderer/pipeline.h"
#include "engine/filesystem.h"
#include "utils/log.h"

namespace gpr5300
{

void Shader::LoadShader(const pb::Shader &shader)
{
    GLenum glType = 0;
    switch (shader.type())
    {
    case pb::Shader_Type_VERTEX:
        glType = GL_VERTEX_SHADER;
        break;
    case pb::Shader_Type_FRAGMENT:
        glType = GL_FRAGMENT_SHADER;
        break;
    case pb::Shader_Type_COMPUTE:
        glType = GL_COMPUTE_SHADER;
        break;
    default:
        break;
    }
    const auto &filesystem = FilesystemLocator::get();
    std::string_view path = shader.path();
    if (filesystem.IsRegularFile(path))
    {
        const auto file = filesystem.LoadFile(path);
        const GLuint shaderName = glCreateShader(glType);

        glShaderSource(shaderName, 1, reinterpret_cast<const GLchar *const *>( &file.data), nullptr);
        glCompileShader(shaderName);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(shaderName, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            constexpr GLsizei infoLogSize = 512;
            char infoLog[infoLogSize];
            glGetShaderInfoLog(shaderName, infoLogSize, nullptr, infoLog);
            LogError(fmt::format("Shader compilation failed with this log:\n{}\nShader Path:\n{}",
                              infoLog, path));
        }
        name = shaderName;
        LogDebug(fmt::format("Successfully loaded shader: {} with name: {}", path, name));
    }
    else
    {
        LogError(fmt::format("File not found: {}", path));
    }
}

void Shader::Destroy()
{
    if (name == 0)
    {
        return;
    }
    glDeleteShader(name);
    name = 0;
}

void Pipeline::LoadRasterizePipeline(const Shader &vertex, const Shader &fragment)
{
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex.name);
    glAttachShader(program, fragment.name);

    glLinkProgram(program);
    //Check if shader program was linked correctly
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        constexpr GLsizei infoLogSize = 512;
        char infoLog[infoLogSize];
        glGetProgramInfoLog(program, infoLogSize, nullptr, infoLog);
        LogError(fmt::format("Shader program with vertex {} and fragment {}: LINK_FAILED with infoLog:\n{}",
                          vertex.name,
                          fragment.name,
                          infoLog));
        name = 0;
        return;
    }
    name = program;
    LogDebug(fmt::format("Successfully loaded program with vertex {} and fragment {}", vertex.name, fragment.name));
}

void Pipeline::LoadComputePipeline(const Shader &compute)
{
    const GLuint program = glCreateProgram();
    glAttachShader(program, compute.name);

    glLinkProgram(program);
    //Check if shader program was linked correctly
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        constexpr GLsizei infoLogSize = 512;
        char infoLog[infoLogSize];
        glGetProgramInfoLog(program, infoLogSize, nullptr, infoLog);
        LogError(fmt::format("Shader program with compute {}: LINK_FAILED with infoLog:\n{}",
                          compute.name,
                          infoLog));
        name = 0;
    }
    name = program;
}

void Pipeline::Destroy()
{
    if (name == 0)
    {
        return;
    }
    glDeleteProgram(name);
    name = 0;
}

}
