#pragma once

#include "renderer/draw_command.h"
#include <unordered_map>

#include "material.h"


namespace gl
{
class Pipeline;
class Texture;

class DrawCommand : public core::DrawCommand
{
public:
    DrawCommand(const core::pb::DrawCommand& drawCommandInfo, int subpassIndex);
    void SetFloat(std::string_view uniformName, float f) override;
    void SetInt(std::string_view uniformName, int i) override;
    void SetVec2(std::string_view uniformName, glm::vec2 v) override;
    void SetVec3(std::string_view uniformName, glm::vec3 v) override;
    void SetVec4(std::string_view uniformName, glm::vec4 v) override;
    void SetMat3(std::string_view uniformName, const glm::mat3& mat) override;
    void SetMat4(std::string_view uniformName, const glm::mat4& mat) override;
    void SetAngle(std::string_view uniformName, core::Radian angle) override;
    void SetBool(std::string_view uniformName, bool i) override;

    void SetTexture(std::string_view uniformName, const Texture& texture, GLenum textureUnit);
    void SetTexture(std::string_view uniformName, GLuint textureName, GLenum textureUnit);
    void SetCubemap(std::string_view uniformName, GLuint textureName, GLenum textureUnit);

    void Bind() override;
    void PreDrawBind() override;
private:

    Pipeline* pipeline_ = nullptr;
    Material* material_ = nullptr;
};

class ComputeCommand : public core::ComputeCommand
{
public:
    ComputeCommand(const core::pb::ComputeCommand& computeCommandInfo, int subpassIndex);
    void SetFloat(std::string_view uniformName, float f) override;
    void SetInt(std::string_view uniformName, int i) override;
    void SetBool(std::string_view uniformName, bool i) override;
    void SetVec2(std::string_view uniformName, glm::vec2 v) override;
    void SetVec3(std::string_view uniformName, glm::vec3 v) override;
    void SetVec4(std::string_view uniformName, glm::vec4 v) override;
    void SetMat3(std::string_view uniformName, const glm::mat3& mat) override;
    void SetMat4(std::string_view uniformName, const glm::mat4& mat) override;
    void SetAngle(std::string_view uniformName, core::Radian angle) override;
private:

    Pipeline* pipeline_ = nullptr;
    Material* material_ = nullptr;
};

}
