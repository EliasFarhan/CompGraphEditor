#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <string_view>
#include <string>


namespace core
{
    
class DrawCommand
{
public:
    //Uniform functions
    virtual void SetFloat(std::string_view uniformName, float f) = 0;

    virtual void SetInt(std::string_view uniformName, int i)  = 0;

    virtual void SetVec2(std::string_view uniformName, glm::vec2 v) = 0;

    virtual void SetVec3(std::string_view uniformName, glm::vec3 v) = 0;

    virtual void SetVec4(std::string_view uniformName, glm::vec4 v) = 0;

    virtual void SetMat4(std::string_view uniformName, const glm::mat4& mat) = 0;

    void SetName(std::string_view name){name_ = name;}
    [[nodiscard]] std::string_view GetName() const{return name_;}

private:
    std::string name_;
};

} // namespace core
