#pragma once

#include <string_view>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace core
{

class Shader
{

};

class Pipeline
{
public:
    virtual ~Pipeline() = default;
    virtual void Bind() = 0;

    //Uniform functions
    virtual void SetFloat(std::string_view uniformName, float f) = 0;

    virtual void SetInt(std::string_view uniformName, int i)  = 0;

    virtual void SetVec2(std::string_view uniformName, glm::vec2 v) = 0;

    virtual void SetVec3(std::string_view uniformName, glm::vec3 v) = 0;

    virtual void SetVec4(std::string_view uniformName, glm::vec4 v) = 0;

    virtual void SetMat4(std::string_view uniformName, const glm::mat4& mat) = 0;
    void SetPipelineName(std::string_view name);
    [[nodiscard]] std::string_view GetPipelineName() const;
private:
    std::string pipelineName_;

};
} // namespace core
