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

    switch (channelInFile)
    {
    case 1:
    {
        image = engine.CreateImage(width, height, VK_FORMAT_R8_SRGB, 1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipMapLevels);
        break;
    }
    case 2:
    {
        image = engine.CreateImage(width, height, VK_FORMAT_R8G8_SRGB, 1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipMapLevels);
        break;
    }
    case 3:
    {
        image = engine.CreateImage(width, height, VK_FORMAT_R8G8B8_SRGB, 1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipMapLevels);
        break;
    }
    case 4:
    {
        image = engine.CreateImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, 1,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mipMapLevels);
        break;
    }
    default:
        LogError(fmt::format("Invalid channel count on image. Count: {}, for texture at path: {}", channelInFile, path));
        return false;
    }

    

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
    return true;
}

bool Texture::LoadCubemap(const core::pb::Texture& texture)
{
    return false;
}

void Texture::Destroy()
{
    const auto& allocator = GetAllocator();
    vmaDestroyImage(allocator, image.image, image.allocation);
    image.image = VK_NULL_HANDLE;
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
}
