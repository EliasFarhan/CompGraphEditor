#include "novus/texture.h"

#include "novus/engine.h"

#include "novus/texture.h"

#include <stb_image.h>

namespace novus
{
core::TextureId TextureManager::LoadTexture(const renderer::TextureT& textureInfo)
{
    auto* sampler = GenerateSampler(*textureInfo.sampler);
    SDL_GPUTextureCreateInfo createInfo{
        .type = static_cast<SDL_GPUTextureType>(textureInfo.info->type),
        .format = static_cast<SDL_GPUTextureFormat>(textureInfo.info->format),
        .usage = textureInfo.info->usage,
        .width = textureInfo.info->width,
        .height = textureInfo.info->height,
        .layer_count_or_depth = textureInfo.info->layer_count_or_depth,
        .num_levels = textureInfo.info->num_levels,
        .sample_count = static_cast<SDL_GPUSampleCount>(textureInfo.info->sample_count),
        };
    auto* texture = SDL_CreateGPUTexture(GetDevice(), &createInfo);
    core::TextureId textureId = {static_cast<core::TextureId>(textures_.size())};
    textures_.push_back({.texture = texture,
        .sampler = sampler,
        .path = textureInfo.path,
        .requiredChannels = 4});
	return textureId;
}
const Texture& TextureManager::GetTexture(core::TextureId textureId) const
{
    return textures_[static_cast<int>(textureId)];
}

void TextureManager::Clear()
{
    for (auto& texture : textures_)
    {
        SDL_ReleaseGPUTexture(GetDevice(), texture.texture);
    }
    for (auto& samplerMapping: sampleMap_)
    {
        SDL_ReleaseGPUSampler(GetDevice(), samplerMapping.second);
    }
    textures_.clear();
    sampleMap_.clear();

}
void TextureManager::UploadTextures()
{
    for (auto& texture : textures_)
    {
        Image image{};
        image.buffer =image.buffer = stbi_load(texture.path.data(),
        &image.width,
        &image.height,
        &image.channels,
        texture.requiredChannels);
        if(image.buffer == nullptr)
        {
            throw std::runtime_error(std::format("Could not load texture: {}", texture.path));
        }
        image.length = image.width * image.height * (texture.requiredChannels == 0 ? image.channels : texture.requiredChannels);
        image.transferBuffer = GenerateTransferBuffer(image.length, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD);
        image.transferBuffer.UploadBuffer(image.buffer, image.length);
        stbi_image_free(image.buffer);
        image.buffer = nullptr;
        images_.push_back(image);
    }
    auto* commandBuffer = SDL_AcquireGPUCommandBuffer(GetDevice());
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
    for (int64_t i = 0; i < std::ssize(textures_); i++)
    {
        auto& image = images_[i];
        auto& texture = textures_[i];
        SDL_GPUTextureTransferInfo source
                {
                    .transfer_buffer = image.transferBuffer.get(),
                    .offset = 0,
                    .pixels_per_row = 0,
                    .rows_per_layer = 0
                    };
        SDL_GPUTextureRegion destination
            {
                .texture = texture.texture,
                .mip_level = 0,
                .layer = 0,
                .x = 0,
                .y = 0,
                .z = 0,
                .w = static_cast<uint32_t>(image.width),
                .h = static_cast<uint32_t>(image.height),
                .d = 1
                };
        SDL_UploadToGPUTexture(copyPass, &source, &destination, false);
    }
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    for (auto& image: images_)
    {
        image.transferBuffer.Destroy();
    }
    images_.clear();
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
