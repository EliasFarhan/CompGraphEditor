#pragma once

#include "renderer/texture.h"

namespace vk
{
class TextureManager : public core::TextureManager
{
public:
    core::TextureId LoadTexture(const core::pb::Texture &textureInfo) override;
    void Clear() override;
};
}