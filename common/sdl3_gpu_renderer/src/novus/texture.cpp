#include "novus/texture.h"

#include "novus/engine.h"

namespace novus
{
core::TextureId TextureManager::LoadTexture(const renderer::TextureT& textureInfo)
{

    auto* sampler = GenerateSampler(*textureInfo.sampler);
	return {};
}
const Texture& TextureManager::GetTexture(core::TextureId textureId) const
{
    return textures_[static_cast<int>(textureId)];
}

void TextureManager::Clear()
{
    
}
SDL_GPUSampler* TextureManager::GenerateSampler(internal::SamplerInfoT& samplerInfo)
{
    SamplerInfo newSamplerInfo{.mipLodBias = samplerInfo.mip_lod_bias,
        .maxAnisotropy = samplerInfo.max_anisotropy,
        .minLod = samplerInfo.min_lod,
        .maxLod = samplerInfo.max_lod,
        .bools = {.value =
            {.compareOp = samplerInfo.compare_op, .addressModeU = samplerInfo.address_mode_u,
            .addressModeV = samplerInfo.address_mode_v, .addressModeW = samplerInfo.address_mode_w,
            .minFilter = samplerInfo.min_filter, .magFilter = samplerInfo.mag_filter,
            .mipmapMode = samplerInfo.mipmap_mode, .enableAnisotropy = samplerInfo.enable_anisotropy,
            .enableCompare = samplerInfo.enable_compare}}};

    auto it = sampleMap_.find(newSamplerInfo);
    if (it != sampleMap_.end())
    {
        return it->second;
    }
    SDL_GPUSamplerCreateInfo createInfo{.min_filter = (SDL_GPUFilter)samplerInfo.min_filter,
        .mag_filter = (SDL_GPUFilter)samplerInfo.mag_filter,
        .mipmap_mode = (SDL_GPUSamplerMipmapMode)samplerInfo.mipmap_mode,
        .address_mode_u = (SDL_GPUSamplerAddressMode)samplerInfo.address_mode_u,
        .address_mode_v = (SDL_GPUSamplerAddressMode)samplerInfo.address_mode_v,
        .address_mode_w = (SDL_GPUSamplerAddressMode)samplerInfo.address_mode_w,
        .mip_lod_bias = samplerInfo.mip_lod_bias, .max_anisotropy = samplerInfo.max_anisotropy,
        .compare_op = (SDL_GPUCompareOp)samplerInfo.compare_op,
        .min_lod = samplerInfo.min_lod,
        .max_lod = samplerInfo.max_lod,
        .enable_anisotropy = samplerInfo.enable_anisotropy,
        .enable_compare = samplerInfo.enable_compare};
    auto* sampler = SDL_CreateGPUSampler(GetDevice(), &createInfo);
    if (sampler ==nullptr)
    {
        throw std::runtime_error("SDL_CreateGPUSampler failed");
    }
    sampleMap_.insert(std::make_pair(newSamplerInfo, sampler));
    return sampler;
}
} // namespace novus
