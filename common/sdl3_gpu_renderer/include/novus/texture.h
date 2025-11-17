//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_TEXTURE_H
#define NEKO2_TEXTURE_H

#include "renderer/texture.h"
#include <SDL3/SDL_gpu.h>
#include "generated/renderer_generated.h"

struct MyHash
{
    std::size_t operator()(const novus::internal::SamplerInfoT& s) const noexcept
    {

        return 0; // or use boost::hash_combine
    }
};

namespace novus
{
struct Texture
{
    SDL_GPUTexture* texture = nullptr;
    SDL_GPUSampler* sampler = nullptr;
};
class TextureManager : public core::TextureManager
{
public:
	core::TextureId LoadTexture(const renderer::TextureT& textureInfo) override;
    const Texture& GetTexture(core::TextureId textureId);
	void Clear() override;
private:
    std::unordered_map<internal::SamplerInfoT, SDL_GPUSampler*> sampleMap_;

};
}
#endif //NEKO2_TEXTURE_H