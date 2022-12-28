#pragma once

#include "renderer/texture.h"

namespace gpr5300::vk
{
class TextureManager : public gpr5300::TextureManager
{
public:
    TextureId LoadTexture(const pb::Texture &textureInfo) override;

};
}