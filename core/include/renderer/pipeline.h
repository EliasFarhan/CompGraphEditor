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

    void SetPipelineName(std::string_view name);
    [[nodiscard]] std::string_view GetPipelineName() const;
private:
    std::string pipelineName_;

};
} // namespace core
