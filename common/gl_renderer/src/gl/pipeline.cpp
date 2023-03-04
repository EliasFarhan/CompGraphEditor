#include "engine/filesystem.h"
#include "gl/pipeline.h"
#include "gl/debug.h"
#include "gl/texture.h"
#include "utils/log.h"


#include <fmt/format.h>


namespace gl
{
Shader::~Shader()
{
    if(name != 0)
    {
        LogWarning("Forgot to clear Shader");
    }
}

void Shader::LoadShader(const core::pb::Shader &shader)
{
    GLenum glType = 0;
    switch (shader.type())
    {
    case core::pb::Shader_Type_VERTEX:
        glType = GL_VERTEX_SHADER;
        break;
    case core::pb::Shader_Type_FRAGMENT:
        glType = GL_FRAGMENT_SHADER;
        break;
    case core::pb::Shader_Type_COMPUTE:
        glType = GL_COMPUTE_SHADER;
        break;
    default:
        break;
    }
    const auto &filesystem = core::FilesystemLocator::get();
    std::string_view path = shader.path();
    if (filesystem.FileExists(path))
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
            glDeleteShader(shaderName);
            return;
        }
        name = shaderName;
        LogDebug(fmt::format("Successfully loaded shader: {} with name: {}", path, name));
    }
    else
    {
        LogError(fmt::format("File not found: {}", path));
    }
    glCheckError();
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

Pipeline::~Pipeline()
{
    if(name != 0)
    {
        LogWarning("Forgot to clear Pipeline");
    }
}

void Pipeline::Bind()
{
    if (currentBindedPipeline != name)
    {
        glUseProgram(name);
        glCheckError();
        currentBindedPipeline = name;
    }
}

void Pipeline::Unbind()
{
    glUseProgram(0);
    currentBindedPipeline = 0;
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
    glCheckError();
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
    if(currentBindedPipeline == name)
    {
        Unbind();
    }
    glDeleteProgram(name);
    name = 0;
}

void Pipeline::SetFloat(std::string_view uniformName, float f)
{
    Bind();
    glUniform1f(GetUniformLocation(uniformName), f);
    glCheckError();
}

void Pipeline::SetInt(std::string_view uniformName, int i)
{
    Bind();
    glUniform1i(GetUniformLocation(uniformName), i);
    glCheckError();
}

void Pipeline::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    Bind();
    glUniform2fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    Bind();
    glUniform3fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    Bind();
    glUniform4fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    Bind();
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void Pipeline::SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(texture.target, texture.name);
    glCheckError();
}

void Pipeline::SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glCheckError();
}

void Pipeline::SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);
    glCheckError();
}


int Pipeline::GetUniformLocation(std::string_view uniformName)
{
    const auto uniformIt = uniformMap_.find(uniformName.data());
    GLint uniformLocation;
    if (uniformIt == uniformMap_.end())
    {
        uniformLocation = glGetUniformLocation(name, uniformName.data());
        glCheckError();
        uniformMap_[uniformName.data()] = uniformLocation;
    }
    else
    {
        uniformLocation = uniformIt->second;
    }
    return uniformLocation;
}
}
