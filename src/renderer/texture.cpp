#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "renderer/texture.h"
#include "engine/filesystem.h"
#include "utils/log.h"

#include <fmt/format.h>
#include "renderer/debug.h"

#include <ktx.h>


namespace gpr5300
{

Texture TextureManager::LoadTexture(const pb::Texture &textureInfo)
{
    const auto& path = textureInfo.path();
    const auto it = texturesMap_.find(path);
    if(it == texturesMap_.end())
    {
        Texture newTexture;
        if(path.find(".cube") != std::string::npos)
        {
            if(!newTexture.LoadCubemap(textureInfo))
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
            glTexImage2D(GL_TEXTURE_2D, 0, textureInfo.gamma_correction()?GL_SRGB8:GL_RGB8, width, height,
                         0,
                         GL_RGB, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, textureInfo.gamma_correction()?GL_SRGB8_ALPHA8:GL_RGBA8, width, height,
                         0,
                         GL_RGBA, GL_UNSIGNED_BYTE,
                         imageData);
            break;
        default:
            LogError(fmt::format("Invalid channel count on image. Count: {}, for texture at path: {}", channelInFile, path));
            return false;
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

bool Texture::LoadCubemap(const pb::Texture& textureInfo)
{
    target = GL_TEXTURE_CUBE_MAP;
    stbi_set_flip_vertically_on_load(false);
    const auto& filesystem = FilesystemLocator::get();
    std::string_view path = textureInfo.path();

    if (filesystem.FileExists(path))
    {
        pb::Cubemap cubemap;
        const auto file = filesystem.LoadFile(path);
        if(!cubemap.ParseFromArray(file.data, file.length))
        {
            LogError(fmt::format("Could not open proto of cubemap at: {}", path));
            return false;
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
        glBindTexture(GL_TEXTURE_CUBE_MAP, name);

        GLint wrappingMode = GL_REPEAT;
        switch (textureInfo.wrapping_mode())
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

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrappingMode);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrappingMode);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrappingMode);

        int minFilterMode = GL_NEAREST;
        int magFilterMode = GL_NEAREST;
        switch (textureInfo.filter_mode())
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

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilterMode);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilterMode);

        int width = 0, height = 0, nrChannels = 0;
        unsigned char* data = nullptr;
        for(int i = 0; i < cubemap.texture_paths_size(); i++)
        {
            const auto cubeTextureFile = filesystem.LoadFile(cubemap.texture_paths(i));
            data = stbi_load_from_memory(cubeTextureFile.data, cubeTextureFile.length, &width, &height, &nrChannels, 0);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textureInfo.gamma_correction()?GL_SRGB:GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        return true;
    }

    LogError(fmt::format("File not found at path: {}", path));
    return false;
}

bool Texture::LoadKtxTexture(const pb::Texture& textureInfo)
{
    ktxTexture* kTexture;
    ktx_size_t offset;
    ktx_uint8_t* image;
    ktx_uint32_t level, layer, faceSlice;
    GLenum glerror;

    KTX_error_code result = ktxTexture_CreateFromNamedFile("mytex3d.ktx",
                                                           KTX_TEXTURE_CREATE_NO_FLAGS,
                                                           &kTexture);
    if(!CheckKtxError(result))
    {
        return false;
    }
    glGenTextures(1, &name); // Optional. GLUpload can generate a texture.
    result = ktxTexture_GLUpload(kTexture, &name, &target, &glerror);
    if (!CheckKtxError(result))
    {
        if(result == KTX_GL_ERROR)
        {
            LogError(GetGlError(glerror));
        }
        return false;
    }
    ktxTexture_Destroy(kTexture);

    return true;
}

void Texture::Destroy()
{
    glDeleteTextures(1, &name);
}
} // namespace gpr5300
