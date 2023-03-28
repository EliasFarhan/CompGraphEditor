#include "gl/draw_command.h"
#include "gl/texture.h"

#include "engine/scene.h"
#include "gl/material.h"
#include "gl/pipeline.h"
#include "proto/renderer.pb.h"

#include <glm/gtx/euler_angles.hpp>

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
    pipeline_->SetFloat(uniformName, f);
}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    pipeline_->SetInt(uniformName, i);
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    pipeline_->SetVec2(uniformName, v);
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    pipeline_->SetVec3(uniformName, v);
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    pipeline_->SetVec4(uniformName, v);
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    pipeline_->SetMat4(uniformName, mat);
}

void DrawCommand::SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit)
{
    pipeline_->SetTexture(uniformName, texture, textureUnit);
}

void DrawCommand::SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    pipeline_->SetTexture(uniformName, textureName, textureUnit);
}

void DrawCommand::SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit)
{
    pipeline_->SetCubemap(uniformName, textureName, textureUnit);
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
    if(drawCommandInfo_.get().has_model_transform())
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        const auto& modelTransform = drawCommandInfo_.get().model_transform();
        if(modelTransform.has_euler_angles())
        {
            const auto& eulerAngles = modelTransform.euler_angles();
            modelMatrix *= glm::orientate4(glm::vec3(eulerAngles.x(), eulerAngles.y(), eulerAngles.z()));
        }
        if(modelTransform.has_scale())
        {
            const auto& scale = modelTransform.scale();
            modelMatrix = glm::scale(modelMatrix, glm::vec3(scale.x(), scale.y(), scale.z()));
        }
        if(modelTransform.has_position())
        {
            const auto& translate = modelTransform.position();
            modelMatrix = glm::translate(modelMatrix, glm::vec3(translate.x(), translate.y(), translate.z()));
        }
        SetMat4("model", modelMatrix);
    }
}

void DrawCommand::PreDrawBind()
{
    //TODO bind transform if any
}
}
