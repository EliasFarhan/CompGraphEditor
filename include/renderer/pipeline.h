#pragma once

#include <string_view>
#include <GL/glew.h>

namespace gpr5300
{

struct Shader
{
    enum class ShaderType
    {
        VERTEX,
        FRAGMENT,
        COMPUTE,
        NONE
    };

    void LoadShader(std::string_view path, ShaderType shaderType);


    ShaderType shaderType_ = ShaderType::NONE;
    GLuint name_ = 0;
};

class Pipeline
{
public:

private:
    
};

}