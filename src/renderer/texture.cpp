#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "renderer/texture.h"
#include "engine/filesystem.h"
#include "utils/log.h"

#include <fmt/format.h>
#include "renderer/debug.h"


namespace gpr5300
{

Texture TextureManager::LoadTexture(const pb::Texture &textureInfo)
{
    const auto& path = textureInfo.path();
    const auto it = texturesMap_.find(path);
    if(it == texturesMap_.end())
    {
        Texture newTexture;
        if(!newTexture.LoadTexture(textureInfo))
        {
            return {};
        }
        texturesMap_[path] = newTexture;
        return newTexture;
    }
    return it->second;
}

bool Texture::LoadTexture(const pb::Texture &textureInfo)
{
    stbi_set_flip_vertically_on_load(true);
    const auto &filesystem = FilesystemLocator::get();
    std::string_view path = textureInfo.path();
    if (filesystem.FileExists(path))
    {
        const auto file = filesystem.LoadFile(path);
        int channelInFile;
        const auto* imageData = stbi_load_from_memory(file.data, file.length, &width, &height, &channelInFile, 0);
        if(imageData == nullptr)
        {
            LogError(fmt::format("Could not decode image from path: {}", path));
            return false;
        }
        glGenTextures(1, &name);
        glCheckError();

        glBindTexture(GL_TEXTURE_2D, name);

        GLint wrappingMode = GL_REPEAT;
        switch(textureInfo.wrapping_mode())
        {
        case pb::Texture_WrappingMode_REPEAT: 
            wrappingMode = GL_REPEAT;
            break;
        case pb::Texture_WrappingMode_MIRROR_REPEAT: 
            wrappingMode = GL_MIRRORED_REPEAT;
            break;
        case pb::Texture_WrappingMode_CLAMP_TO_EDGE: 
            wrappingMode = GL_CLAMP_TO_EDGE;
            break;
        case pb::Texture_WrappingMode_CLAMP_TO_BORDER: 
            wrappingMode = GL_CLAMP_TO_BORDER;
            break;
        default: 
            break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMode);

        int minFilterMode = GL_NEAREST;
        int magFilterMode = GL_NEAREST;
        switch(textureInfo.filter_mode())
        {
        case pb::Texture_FilteringMode_LINEAR:
        {
            magFilterMode = GL_LINEAR;
            minFilterMode = textureInfo.generate_mipmaps() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            break;
        }
        case pb::Texture_FilteringMode_NEAREST:
        {
            magFilterMode = GL_NEAREST;
            minFilterMode = textureInfo.generate_mipmaps() ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST;
            break;
        }
        default:
            break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterMode);

        switch (channelInFile)
        {
        case 1:
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height,
                         0,
                         GL_RED, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        }
        case 2:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height,
                         0,
                         GL_RG, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height,
                         0,
                         GL_RGB, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
                         0,
                         GL_RGBA, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        default:
            LogWarning(fmt::format("Weird channel count on image. Count: {}", channelInFile));
            break;
        }
        glCheckError();
        if(textureInfo.generate_mipmaps())
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        LogDebug(fmt::format("Successfully loaded texture at path: {}", path));
        return true;
    }
    LogError(fmt::format("File not found at path: {}", path));
    return false;
}
} // namespace gpr5300
