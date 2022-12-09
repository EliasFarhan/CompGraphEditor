#pragma once

#include "renderer/texture.h"
#include "renderer/material.h"


namespace gpr5300::gl
{

struct MaterialTexture
{
    TextureId textureId;
    std::string uniformSamplerName;
    std::string attachmentName;
    std::string framebufferName;
};

struct Material : gpr5300::Material
{
    std::string name;
    int pipelineIndex = -1;
    std::vector<MaterialTexture> textures;
    [[nodiscard]] std::string_view GetName() const override { return name; }
};

}