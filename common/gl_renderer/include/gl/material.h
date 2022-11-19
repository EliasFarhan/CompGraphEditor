#pragma once

#include "renderer/texture.h"


namespace gpr5300::gl
{

struct MaterialTexture
{
    Texture texture;
    std::string uniformSamplerName;
    std::string attachmentName;
    std::string framebufferName;
};

struct Material
{
    std::string name;
    int pipelineIndex = -1;
    std::vector<MaterialTexture> textures;
};

}