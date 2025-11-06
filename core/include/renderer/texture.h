#pragma once
#include "generated/renderer_generated.h"


namespace core
{
enum class TextureId : int {};
constexpr TextureId INVALID_TEXTURE_ID = TextureId{ -1 };

class Texture
{
public:
    virtual ~Texture() = default;
    virtual bool LoadTexture(const novus::renderer::TextureT& texture) = 0;
    virtual bool LoadCubemap(const novus::renderer::TextureT& texture) = 0;
};

class TextureManager
{
public:
    virtual ~TextureManager() = default;
    virtual TextureId LoadTexture(const novus::renderer::TextureT& textureInfo) = 0;
    virtual void Clear() = 0;
};
} // namespace core
