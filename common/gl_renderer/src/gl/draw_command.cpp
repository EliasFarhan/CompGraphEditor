#include "gl/draw_command.h"
#include "gl/texture.h"

#include "engine/scene.h"
#include "gl/material.h"
#include "gl/pipeline.h"
#include "proto/renderer.pb.h"

namespace gl
{
DrawCommand::DrawCommand(const core::pb::DrawCommand& drawCommandInfo, int subpassIndex) : core::DrawCommand(drawCommandInfo, subpassIndex)
{
    auto* scene = core::GetCurrentScene();
    const auto material = scene->GetMaterial(GetMaterialIndex());
    material_ = static_cast<Material*>(material.GetMaterial());
    pipeline_ = static_cast<Pipeline*>(material.GetPipeline());
}

void DrawCommand::SetFloat(std::string_view uniformName, float f)
{
    pipeline_->Bind();
    glUniform1f(GetUniformLocation(uniformName), f);
    glCheckError();
}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    pipeline_->Bind();
    glUniform1i(GetUniformLocation(uniformName), i);
    glCheckError();
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    pipeline_->Bind();
    glUniform2fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    pipeline_->Bind();
    glUniform3fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    pipeline_->Bind();
    glUniform4fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    pipeline_->Bind();
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, &mat[0][0]);
    glCheckError();
}

void DrawCommand::SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(texture.target, texture.name);
    glCheckError();
}

void DrawCommand::SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glCheckError();
}

void DrawCommand::SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    SetInt(uniformName, textureUnit);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureName);
    glCheckError();
}

void DrawCommand::Bind()
{
    auto& textureManager = static_cast<TextureManager&>(core::GetTextureManager());
    pipeline_->Bind();
    for (std::size_t textureIndex = 0; textureIndex < material_->textures.size(); textureIndex++)
    {
        if (material_->textures[textureIndex].textureId == core::INVALID_TEXTURE_ID)
            continue;
        SetTexture(
            material_->textures[textureIndex].uniformSamplerName,
            textureManager.GetTexture(material_->textures[textureIndex].textureId),
            textureIndex);
    }
}

int DrawCommand::GetUniformLocation(std::string_view uniformName)
{
    const auto uniformIt = uniformMap_.find(uniformName.data());
    GLint uniformLocation;
    if (uniformIt == uniformMap_.end())
    {
        uniformLocation = glGetUniformLocation(pipeline_->GetName(), uniformName.data());
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
