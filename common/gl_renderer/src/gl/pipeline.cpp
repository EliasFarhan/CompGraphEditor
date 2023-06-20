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
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadShader, "Load Shader", true);
#endif
    GLenum glType = 0;
    switch (shader.type())
    {
    case core::pb::VERTEX:
        glType = GL_VERTEX_SHADER;
        break;
    case core::pb::FRAGMENT:
        glType = GL_FRAGMENT_SHADER;
        break;
    case core::pb::COMPUTE:
        glType = GL_COMPUTE_SHADER;
        break;
    case core::pb::GEOMETRY:
        glType = GL_GEOMETRY_SHADER;
        break;
    case core::pb::TESSELATION_CONTROL:
        glType = GL_TESS_CONTROL_SHADER;
        break;
    case core::pb::TESSELATION_EVAL:
        glType = GL_TESS_EVALUATION_SHADER;
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

void Pipeline::LoadRasterizePipeline(
    const Shader &vertex, 
    const Shader &fragment,
    std::optional<std::reference_wrapper<Shader>> geometryShader,
    std::optional<std::reference_wrapper<Shader>> tesselationControlShader,
    std::optional<std::reference_wrapper<Shader>> tesselationEvalShader)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadPipeline, "Load Raterize Pipeline", true);
#endif
    const GLuint program = glCreateProgram();
    auto* scene = core::GetCurrentScene();
    const auto& sceneInfo = core::GetCurrentScene()->GetInfo();
    auto addSsbo = [this](const core::pb::Shader& shaderInfo)
    {
        for (const auto& bufferBinding : shaderInfo.storage_buffers())
        {
            core::BufferBinding binding = {bufferBinding.name(), bufferBinding.binding()};
            bufferBindings_.push_back(binding);
        }
    };
    glAttachShader(program, vertex.name);
    if (scene)
    {
        addSsbo(sceneInfo.shaders(vertex.shaderIndex));
    }
    
    glAttachShader(program, fragment.name);
    if (scene)
    {
        addSsbo(sceneInfo.shaders(fragment.shaderIndex));
    }
    if(geometryShader)
    {
        glAttachShader(program, geometryShader.value().get().name);
        if (scene)
        {
            addSsbo(sceneInfo.shaders(geometryShader.value().get().shaderIndex));
        }
    }
    if(tesselationControlShader)
    {
        glAttachShader(program, tesselationControlShader.value().get().name);
        if (scene)
        {
            addSsbo(sceneInfo.shaders(tesselationControlShader.value().get().shaderIndex));
        }
    }
    if(tesselationEvalShader)
    {
        glAttachShader(program, tesselationEvalShader.value().get().name);
        if (scene)
        {
            addSsbo(sceneInfo.shaders(tesselationEvalShader.value().get().shaderIndex));
        }
    }

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
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(loadPipeline, "Load Compute Pipeline", true);
#endif
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
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Float", true);
#endif
    Bind();
    glUniform1f(GetUniformLocation(uniformName), f);
    glCheckError();
}

void Pipeline::SetInt(std::string_view uniformName, int i)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Int", true);
#endif
    Bind();
    glUniform1i(GetUniformLocation(uniformName), i);
    glCheckError();
}

void Pipeline::SetVec2(std::string_view uniformName, glm::vec2 v)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Vec2", true);
#endif
    Bind();
    glUniform2fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetVec3(std::string_view uniformName, glm::vec3 v)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Vec3", true);
#endif
    Bind();
    glUniform3fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetVec4(std::string_view uniformName, glm::vec4 v)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Vec4", true);
#endif
    Bind();
    glUniform4fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void Pipeline::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Mat4", true);
#endif
    Bind();
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void Pipeline::SetMat3(std::string_view uniformName, const glm::mat3& mat)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Mat3", true);
#endif
    Bind();
    glUniformMatrix3fv(GetUniformLocation(uniformName), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void Pipeline::SetBool(std::string_view uniformName, bool b)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(setUniform, "Set Uniform Bool", true);
#endif
    Bind();
    glUniform1i(GetUniformLocation(uniformName), static_cast<int>(b));
    glCheckError();
}

void Pipeline::SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(bindTexture, "Bind Texture", true);
#endif
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(texture.target, texture.name);
    glCheckError();
}

void Pipeline::SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(bindTexture, "Bind Texture", true);
#endif
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glCheckError();
}

void Pipeline::SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
#ifdef TRACY_ENABLE
    TracyGpuNamedZone(bindTexture, "Bind Cubemap", true);
#endif
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
#ifdef TRACY_ENABLE
        TracyGpuNamedZone(getUniform, "Get Uniform Location", true);
#endif
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
