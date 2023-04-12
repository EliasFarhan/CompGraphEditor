#pragma once

#include "renderer/texture.h"
#include "renderer/material.h"


namespace gl
{

struct MaterialTexture
{
    core::TextureId textureId = core::INVALID_TEXTURE_ID;
    std::string uniformSamplerName;
    std::string attachmentName;
    std::string framebufferName;
};

struct Material : core::Material
{
    std::string name;
    int pipelineIndex = -1;
    std::vector<MaterialTexture> textures;
    [[nodiscard]] std::string_view GetName() const override { return name; }
};

}