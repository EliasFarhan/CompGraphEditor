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
        newTexture.LoadTexture(textureInfo);
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

    }
    LogError(fmt::format("File not found at path: {}", path));
    return false;
}
} // namespace gpr5300
