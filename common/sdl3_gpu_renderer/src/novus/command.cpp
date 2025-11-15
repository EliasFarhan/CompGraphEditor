//
// Created by unite on 10.10.2025.
//
#include "novus/command.h"

#include "engine/scene.h"

namespace novus
{

DrawCommand::DrawCommand(renderer::DrawCommandT drawCommandInfo, int subpassIndex, Pipeline* pipeline,
                         Material* material):
    core::DrawCommand(drawCommandInfo, subpassIndex), pipeline_(pipeline), material_(material)
{
}
void DrawCommand::SetFloat(std::string_view uniformName, float f)
{
    pipeline_->SetUniform(uniformName, &f);
}

void DrawCommand::SetInt(std::string_view uniformName, int i)
{
    pipeline_->SetUniform(uniformName, &i);
}

void DrawCommand::SetBool(std::string_view uniformName, bool i)
{

    pipeline_->SetUniform(uniformName, &i);
}

void DrawCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    pipeline_->SetUniform(uniformName, &v);
}

void DrawCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    pipeline_->SetUniform(uniformName, &v);
}

void DrawCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    pipeline_->SetUniform(uniformName, &v);
}

void DrawCommand::SetMat3(std::string_view uniformName, const glm::mat3& mat)
{
    pipeline_->SetUniform(uniformName, &mat);
}

void DrawCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    pipeline_->SetUniform(uniformName, &mat);
}

void DrawCommand::SetAngle(std::string_view uniformName, core::Radian angle)
{
    pipeline_->SetUniform(uniformName, &angle);
}

void DrawCommand::Bind(void* renderData)
{
    material_->Bind(renderData);
}

void DrawCommand::PreDrawBind(void* renderData)
{
    pipeline_->UploadDirtyUniformData();
}

void ComputeCommand::SetFloat(std::string_view uniformName, float f)
{

}

void ComputeCommand::SetInt(std::string_view uniformName, int i)
{
}

void ComputeCommand::SetBool(std::string_view uniformName, bool i)
{
}

void ComputeCommand::SetVec2(std::string_view uniformName, glm::vec2 v)
{
}

void ComputeCommand::SetVec3(std::string_view uniformName, glm::vec3 v)
{
}

void ComputeCommand::SetVec4(std::string_view uniformName, glm::vec4 v)
{
}

void ComputeCommand::SetMat3(std::string_view uniformName, const glm::mat3& mat)
{
}

void ComputeCommand::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
}

void ComputeCommand::SetAngle(std::string_view uniformName, core::Radian angle)
{
}

void ComputeCommand::Bind(void* renderData)
{
}
} // namespace novus