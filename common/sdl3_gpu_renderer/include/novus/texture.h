//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_TEXTURE_H
#define NEKO2_TEXTURE_H

#include "renderer/texture.h"

namespace novus
{
class TextureManager : public core::TextureManager
{
public:
	core::TextureId LoadTexture(const core::pb::Texture& textureInfo) override;

	void Clear() override;
};
}
#endif //NEKO2_TEXTURE_H