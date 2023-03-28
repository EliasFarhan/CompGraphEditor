//
// Created by efarhan on 12/28/22.
//
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "vk/texture.h"


#include "engine/filesystem.h"
#include "utils/log.h"

#include <fmt/format.h>

#include "vk/engine.h"

namespace vk
{
Texture::~Texture()
{
    if (image.image != VK_NULL_HANDLE)
    {
        LogWarning("Forgot to clear texture");
    }
}

bool Texture::LoadTexture(const core::pb::Texture& textureInfo)
{
    LogDebug(fmt::format("Loading texture: {}", textureInfo.path()));
    stbi_set_flip_vertically_on_load(true);
    const auto& filesystem = core::FilesystemLocator::get();
    std::string_view path = textureInfo.path();
    if (!filesystem.FileExists(path))
    {
        LogError(fmt::format("File not found at path: {}", path));
        return false;
    }
    const auto file = filesystem.LoadFile(path);
    constexpr int requiredChannels = 4;
    int channelInFile;
    auto* imageData = stbi_load_from_memory(file.data, file.length, &width, &height, &channelInFile, requiredChannels);
    if (imageData == nullptr)
    {
        LogError(fmt::format("Could not decode image from path: {}", path));
        return false;
    }
    if(requiredChannels != 0)
    {
        channelInFile = requiredChannels;
    }
    VkDeviceSize imageSize = width * height * channelInFile;
    int mipMapLevels = 1;
    if(textureInfo.generate_mipmaps())
    {
        mipMapLevels = static_cast<std::uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    }

    auto& engine = GetEngine();

    const auto stagingBuffer = engine.CreateBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    const auto& allocator = GetAllocator();
    void* data;
    vmaMapMemory(allocator, stagingBuffer.allocation, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBuffer.allocation);

    stbi_image_free(imageData);

    //TODO manage gamma format
    VkFormat format;
    switch (channelInFile)
    {
    case 1:
    {
        format = VK_FORMAT_R8_SRGB;
        break;
    }
    case 2:
    {
        format = VK_FORMAT_R8G8_SRGB;
        break;
    }
    case 3:
    {
        format = VK_FORMAT_R8G8B8_SRGB;
        break;
    }
    case 4:
    {
        format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }
    default:
        LogError(fmt::format("Invalid channel count on image. Count: {}, for texture at path: {}", channelInFile, path));
        return false;
    }
    image = engine.CreateImage(width, height, format, 1,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipMapLevels);
    

    engine.TransitionImageLayout(image.image, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipMapLevels, 1);

    engine.CopyImageFromBuffer(stagingBuffer, image, width, height, 1);

    vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);

    if(mipMapLevels == 1)
    {
        engine.TransitionImageLayout(image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipMapLevels, 1);
    }
    else
    {
        //TODO Generate the mip maps
    }
    CreateImageView(format, mipMapLevels, 1);
    CreateSampler(textureInfo);
    return true;
}

bool Texture::LoadCubemap(const core::pb::Texture& texture)
{
    return false;
}

void Texture::Destroy()
{
    const auto& allocator = GetAllocator();
    const auto& driver = GetDriver();
    vkDestroySampler(driver.device, sampler, nullptr);
    vkDestroyImageView(driver.device, imageView, nullptr);
    vmaDestroyImage(allocator, image.image, image.allocation);
    image.image = VK_NULL_HANDLE;
}

bool Texture::CreateImageView(VkFormat format, int mipLevels, int layerCount)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(GetDriver().device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        LogError("Failed to create texture image view!");
        return false;
    }
    return true;
}

bool Texture::CreateSampler(const core::pb::Texture& texture)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    switch(texture.filter_mode())
    {
    case core::pb::Texture_FilteringMode_NEAREST:
    {
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        break;
    }
    case core::pb::Texture_FilteringMode_LINEAR:
    {

        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        break;
    }
    default: 
        break;
    }

    switch(texture.wrapping_mode())
    {
    case core::pb::Texture_WrappingMode_REPEAT:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    }
    case core::pb::Texture_WrappingMode_MIRROR_REPEAT:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    }
    case core::pb::Texture_WrappingMode_CLAMP_TO_EDGE:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    }
    case core::pb::Texture_WrappingMode_CLAMP_TO_BORDER:
    {
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        break;
    }
    
    default: 
        break;
    }


    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = GetDriver().maxAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    //TODO Implement mip map
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(GetDriver().device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        LogError("Failed to create texture sampler");
        return false;
    }
    return true;
}

core::TextureId TextureManager::LoadTexture(const core::pb::Texture &textureInfo)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto& path = textureInfo.path();
    const auto it = textureNamesMap_.find(path);
    if (it == textureNamesMap_.end())
    {
        Texture newTexture;
        if (path.find(".cube") != std::string::npos)
        {
            if (!newTexture.LoadCubemap(textureInfo))
            {
                return {};
            }
        }
        else
        {
            if (!newTexture.LoadTexture(textureInfo))
            {
                return {};
            }
        }
        const auto textureId = core::TextureId{ static_cast<int>(textures_.size()) };
        textureNamesMap_[path] = textureId;
        textures_.emplace_back(std::move(newTexture));
        newTexture = {};
        return textureId;
    }
    return it->second;
}

void TextureManager::Clear()
{
    for (auto& texture : textures_)
    {
        texture.Destroy();
    }

    textures_.clear();
    textureNamesMap_.clear();
}

const Texture& TextureManager::GetTexture(core::TextureId textureId) const
{
    return textures_[static_cast<int>(textureId)];
}
}
