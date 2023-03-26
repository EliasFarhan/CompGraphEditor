#pragma once

#include "renderer/draw_command.h"
#include "proto/renderer.pb.h"

namespace vk
{



class DrawCommand : public core::DrawCommand
{
public:
    using core::DrawCommand::DrawCommand;
    void SetFloat(std::string_view uniformName, float f) override;

    void SetInt(std::string_view uniformName, int i) override;

    void SetVec2(std::string_view uniformName, glm::vec2 v) override;

    void SetVec3(std::string_view uniformName, glm::vec3 v) override;

    void SetVec4(std::string_view uniformName, glm::vec4 v) override;

    void SetMat4(std::string_view uniformName, const glm::mat4 &mat) override;

    void Bind() override;

    void PreDrawBind() override;

    void GenerateUniforms();
private:
    struct UniformInternalData
    {
        int index = 0;
        int size = 0;
        bool pushConstant = false;
    };

    std::vector<std::uint8_t> pushConstantBuffer_;
    std::unordered_map<std::string, UniformInternalData> uniforms_;
};
}