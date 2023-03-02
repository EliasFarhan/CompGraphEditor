#pragma once

#include "proto/renderer.pb.h"

namespace core
{
enum class TextureId : int {};
constexpr TextureId INVALID_TEXTURE_ID = TextureId{ -1 };

class Texture
{
public:
    virtual ~Texture() = default;
    virtual bool LoadTexture(const pb::Texture& texture) = 0;
    virtual bool LoadCubemap(const pb::Texture& texture) = 0;
};

class TextureManager
{
public:
    virtual ~TextureManager() = default;
    virtual TextureId LoadTexture(const pb::Texture& textureInfo) = 0;
};
} // namespace core
