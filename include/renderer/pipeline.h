#pragma once

#include <string_view>
#include <GL/glew.h>
#include "proto/renderer.pb.h"

namespace gpr5300
{

struct Shader
{

    void LoadShader(const pb::Shader& shader);
    GLuint name = 0;
    void Destroy();
};

struct Pipeline
{
    GLuint name = 0;

    void LoadRasterizePipeline(const Shader& vertex, const Shader& fragment);
    void LoadComputePipeline(const Shader& compute);
    void Destroy();
};

}