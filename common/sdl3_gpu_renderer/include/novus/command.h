//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_COMMAND_H
#define NEKO2_COMMAND_H
#include "renderer/command.h"

class DrawCommand : public core::DrawCommand
{
public:
    using core::DrawCommand::DrawCommand;
	void SetFloat(std::string_view uniformName, float f) override;

	void SetInt(std::string_view uniformName, int i) override;

	void SetBool(std::string_view uniformName, bool i) override;

	void SetVec2(std::string_view uniformName, glm::vec2 v) override;

	void SetVec3(std::string_view uniformName, glm::vec3 v) override;

	void SetVec4(std::string_view uniformName, glm::vec4 v) override;

	void SetMat3(std::string_view uniformName, const glm::mat3& mat) override;

	void SetMat4(std::string_view uniformName, const glm::mat4& mat) override;

	void SetAngle(std::string_view uniformName, core::Radian angle) override;

	void Bind() override;

	void PreDrawBind() override;
};

class ComputeCommand : public core::ComputeCommand
{
public:
	void SetFloat(std::string_view uniformName, float f) override;

	void SetInt(std::string_view uniformName, int i) override;

	void SetBool(std::string_view uniformName, bool i) override;

	void SetVec2(std::string_view uniformName, glm::vec2 v) override;

	void SetVec3(std::string_view uniformName, glm::vec3 v) override;

	void SetVec4(std::string_view uniformName, glm::vec4 v) override;

	void SetMat3(std::string_view uniformName, const glm::mat3& mat) override;

	void SetMat4(std::string_view uniformName, const glm::mat4& mat) override;

	void SetAngle(std::string_view uniformName, core::Radian angle) override;

	void Bind() override;
};

#endif //NEKO2_COMMAND_H