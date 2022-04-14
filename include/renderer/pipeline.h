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


    ShaderType shaderType = ShaderType::NONE;
    GLuint name = 0;
};

struct Pipeline
{
    GLuint name = 0;

    void LoadRasterizePipeline(Shader* vertex, Shader* fragment);
    void LoadComputePipeline(Shader* compute);
};

}