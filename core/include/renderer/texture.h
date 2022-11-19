#pragma once

#include "proto/renderer.pb.h"

namespace gpr5300
{
class Texture
{

};
class TextureManager
{
public:
    virtual Texture LoadTexture(const pb::Texture& textureInfo);
};
} // namespace gpr5300
