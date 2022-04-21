#pragma once

#include "renderer/pipeline.h"
#include "renderer/texture.h"

#include <span>

namespace gpr5300
{

struct MaterialTexture
{
    Texture texture;
    std::string uniformSamplerName;
};

struct Material
{
    int pipelineIndex = -1;
    std::vector<MaterialTexture> textures;
};

}