#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "renderer/texture.h"


namespace gpr5300
{

Texture gpr5300::TextureManager::LoadTexture(const pb::Texture &textureInfo)
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
    const auto &filesystem = FilesystemLocator::get();
    std::string_view path = textureInfo.path();
    if (filesystem.IsRegularFile(path))
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
        LogDebug(fmt::format("Successfully loaded texture at path: {}", path));
        return true;
    }
    LogError(fmt::format("File not found at path: {}", path));
    return false;
}
} // namespace gpr5300
