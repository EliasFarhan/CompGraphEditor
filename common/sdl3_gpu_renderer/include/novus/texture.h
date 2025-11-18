//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_TEXTURE_H
#define NEKO2_TEXTURE_H

#include "renderer/texture.h"
#include <SDL3/SDL_gpu.h>

#include "novus/buffer.h"
#include "generated/renderer_generated.h"

namespace novus
{

struct SamplerInfo
{
    float mipLodBias;
    float maxAnisotropy;
    float minLod;
    float maxLod;
    union Bools
    {
        struct CompressedSamplerInfo
        {
            int32_t compareOp : 5;
            int32_t addressModeU : 2;
            int32_t addressModeV : 2;
            int32_t addressModeW : 2;
            int32_t minFilter : 1;
            int32_t magFilter : 1;
            int32_t mipmapMode : 1;
            int32_t enableAnisotropy : 1;
            int32_t enableCompare : 1;
        };
        CompressedSamplerInfo value;
        int32_t hash;
    };
    Bools bools;
    constexpr bool operator==(const SamplerInfo& other) const
    {
        return bools.hash == other.bools.hash && mipLodBias == other.mipLodBias && maxAnisotropy == other.maxAnisotropy && minLod == other.minLod && maxLod == other.maxLod;
    }
};

}

struct MyHash
{
    std::size_t operator()(const novus::SamplerInfo& s) const noexcept
    {
        std::size_t hashValue = std::hash<float>{}(s.mipLodBias);
        hashValue = hashValue ^(std::hash<float>{}(s.maxAnisotropy)<<1);
        hashValue = hashValue ^(std::hash<float>{}(s.minLod)<<1);
        hashValue = hashValue ^(std::hash<float>{}(s.maxLod)<<1);
        hashValue = hashValue ^(std::hash<int32_t>{}(s.bools.hash)<<1);
        return hashValue; // or use boost::hash_combine
    }
};

namespace novus
{
struct Image
{
    TransferBuffer transferBuffer;
    uint8_t* buffer = nullptr;
    size_t length = 0;
    int width = -1;
    int height = -1;
    int channels = -1;
};
struct Texture
{
    SDL_GPUTexture* texture = nullptr;
    SDL_GPUSampler* sampler = nullptr;
    std::string path;
    int requiredChannels;
};
class TextureManager : public core::TextureManager
{
public:
	[[nodiscard]] core::TextureId LoadTexture(const renderer::TextureT& textureInfo) override;
    [[nodiscard]] const Texture& GetTexture(core::TextureId textureId) const;
	void Clear() override;
    void UploadTextures();
private:
    SDL_GPUSampler* GenerateSampler(internal::SamplerInfoT& samplerInfo);
    std::unordered_map<SamplerInfo, SDL_GPUSampler*, MyHash> sampleMap_;
    std::vector<Texture> textures_;
    std::vector<Image> images_;
};
}
#endif //NEKO2_TEXTURE_H