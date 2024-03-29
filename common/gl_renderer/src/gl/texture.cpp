#include "gl/texture.h"
#include "engine/filesystem.h"
#include "utils/log.h"

#include "gl/debug.h"

#include <fmt/format.h>
#include <ktx.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <filesystem>

namespace fs = std::filesystem;

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>
#endif

namespace gl
{

core::TextureId TextureManager::LoadTexture(const core::pb::Texture &textureInfo)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    const auto& path = textureInfo.path();
    const auto it = textureNamesMap_.find(path);
    if(it == textureNamesMap_.end())
    {
        Texture newTexture;
        if(path.find(".cube") != std::string::npos)
        {
            if(!newTexture.LoadCubemap(textureInfo))
            {
                return core::INVALID_TEXTURE_ID;
            }
        }
        else if(path.find(".hdr") != std::string::npos)
        {
            if(!newTexture.LoadHdrTexture(textureInfo))
            {
                return core::INVALID_TEXTURE_ID;
            }
        }
        else
        {
            if(path.find(".ktx") != std::string::npos)
            {
                if(!newTexture.LoadKtxTexture(textureInfo))
                {
                    return core::INVALID_TEXTURE_ID;
                }
            }
            else
            {
                if (!newTexture.LoadTexture(textureInfo))
                {
                    return core::INVALID_TEXTURE_ID;
                }
            }
        }
        const auto textureId = core::TextureId{ static_cast<int>(textures_.size()) };
        textureNamesMap_[path] = textureId;
        textures_.emplace_back(std::move(newTexture));
        return textureId;
    }
    return it->second;
}

const Texture& TextureManager::GetTexture(core::TextureId textureId)
{
    return textures_[static_cast<int>(textureId)];
}

void TextureManager::Clear()
{
    for(auto& texture: textures_)
    {
        texture.Destroy();
    }

    textures_.clear();
    textureNamesMap_.clear();
}

Texture::~Texture()
{
    if(name != 0)
    {
        LogWarning("Forgot to clear texture");
    }
}

Texture::Texture(Texture&& other) noexcept
{
    std::swap(other.name, name);
    height = other.height;
    width = other.width;
    target = other.target;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    std::swap(other.name, name);
    height = other.height;
    width = other.width;
    target = other.target;
    return *this;
}

bool Texture::LoadTexture(const core::pb::Texture &textureInfo)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    stbi_set_flip_vertically_on_load(true);
    const auto &filesystem = core::FilesystemLocator::get();
    core::Path path{textureInfo.path()};
    if (filesystem.FileExists(path))
    {
#ifdef TRACY_ENABLE
        TracyCZoneN(ctx, "Load File", true);
#endif
        const auto file = filesystem.LoadFile(path);
#ifdef TRACY_ENABLE
        TracyCZoneEnd(ctx);

        TracyCZoneN(ctx2, "Decompress", true);
#endif
        int channelInFile;
        const auto* imageData = stbi_load_from_memory(file.data, file.size, &width, &height, &channelInFile, 0);
#ifdef TRACY_ENABLE
        TracyCZoneEnd(ctx2);
#endif
        if(imageData == nullptr)
        {
            LogError(fmt::format("Could not decode image from path: {}", path.c_str()));
            return false;
        }
#ifdef TRACY_ENABLE
        ZoneNamedN(gpuUpload, "Upload to GPU", true);
#endif
        glGenTextures(1, &name);
        glCheckError();

        glBindTexture(GL_TEXTURE_2D, name);

        GLint wrappingMode = GL_REPEAT;
        switch(textureInfo.wrapping_mode())
        {
        case core::pb::Texture_WrappingMode_REPEAT: 
            wrappingMode = GL_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_MIRROR_REPEAT: 
            wrappingMode = GL_MIRRORED_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_EDGE: 
            wrappingMode = GL_CLAMP_TO_EDGE;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_BORDER: 
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
        case core::pb::Texture_FilteringMode_LINEAR:
        {
            magFilterMode = GL_LINEAR;
            minFilterMode = textureInfo.generate_mipmaps() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            break;
        }
        case core::pb::Texture_FilteringMode_NEAREST:
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
#ifdef TRACY_ENABLE
        TracyGpuNamedZone(loadTexture, "Load Texture", true);
#endif
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
            LogError(fmt::format("Invalid channel count on image. Count: {}, for texture at path: {}", channelInFile, path.c_str()));
            return false;
        }
        glCheckError();
        if(textureInfo.generate_mipmaps())
        {
#ifdef TRACY_ENABLE
            TracyGpuNamedZone(generateMipMap, "Generate MipMap", true);
#endif
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        LogDebug(fmt::format("Successfully loaded texture at path: {}", path.c_str()));
        return true;
    }
    LogError(fmt::format("File not found at path: {}", path.c_str()));
    return false;
}

bool Texture::LoadCubemap(const core::pb::Texture& textureInfo)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    target = GL_TEXTURE_CUBE_MAP;
    stbi_set_flip_vertically_on_load(false);
    const auto& filesystem = core::FilesystemLocator::get();
    core::Path path{textureInfo.path()};

    if (filesystem.FileExists(path))
    {
        core::pb::Cubemap cubemap;
        const auto file = filesystem.LoadFile(path);
        if(!cubemap.ParseFromArray(file.data, file.size))
        {
            LogError(fmt::format("Could not open proto of cubemap at: {}", path.c_str()));
            return false;
        }

        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name);
        glBindTexture(GL_TEXTURE_CUBE_MAP, name);

        GLint wrappingMode = GL_REPEAT;
        switch (textureInfo.wrapping_mode())
        {
        case core::pb::Texture_WrappingMode_REPEAT:
            wrappingMode = GL_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_MIRROR_REPEAT:
            wrappingMode = GL_MIRRORED_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_EDGE:
            wrappingMode = GL_CLAMP_TO_EDGE;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_BORDER:
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
        case core::pb::Texture_FilteringMode_LINEAR:
        {
            magFilterMode = GL_LINEAR;
            minFilterMode = textureInfo.generate_mipmaps() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            break;
        }
        case core::pb::Texture_FilteringMode_NEAREST:
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
            const core::Path texturePath{cubemap.texture_paths(i)};
            LogDebug(fmt::format("Loading texture side: {}", texturePath));
            const auto cubeTextureFile = filesystem.LoadFile(texturePath);
            data = stbi_load_from_memory(cubeTextureFile.data, cubeTextureFile.size, &width, &height, &nrChannels, 0);
            if(data == nullptr)
            {
                LogError(fmt::format("Could not parse side texture: {} for cubemap: {}", texturePath, path));
                return false;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textureInfo.gamma_correction()?GL_SRGB:GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        return true;
    }

    LogError(fmt::format("File not found at path: {}", path));
    return false;
}

bool Texture::LoadKtxTexture(const core::pb::Texture& textureInfo)
{

#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    ktxTexture* kTexture;
    GLenum glerror;

    const auto& filesystem = core::FilesystemLocator::get();
    core::Path path {textureInfo.path()};

    if (!filesystem.FileExists(path))
    {
        LogError(fmt::format("File not found at path: {}", path));
        return false;
    }

    const auto file = filesystem.LoadFile(path);
    if(file.data == nullptr)
    {
        return false;
    }
    KTX_error_code result = ktxTexture_CreateFromMemory(file.data, file.size,
                                                        KTX_TEXTURE_CREATE_NO_FLAGS,
                                                        &kTexture);
    if(!ktxCheckError(result))
    {
        return false;
    }
    glGenTextures(1, &name); // Optional. GLUpload can generate a texture.
    if(kTexture->classId == ktxTexture2_c)
    {
        LogDebug(fmt::format("Loading KTX2 texture: {}", path));
    }
    else
    {
        LogDebug(fmt::format("Loading KTX1 texture: {}", path));
    }
    result = ktxTexture_GLUpload(kTexture, &name, &target, &glerror);
    
    if (!ktxCheckError(result))
    {
        if(result == KTX_GL_ERROR)
        {
            LogError(GetGlError(glerror));
        }
        glDeleteTextures(1, &name);
        name = 0;
        return false;
    }

    width = kTexture->baseWidth;
    height = kTexture->baseHeight;

    ktxTexture_Destroy(kTexture);

    GLint wrappingMode = GL_REPEAT;
    switch (textureInfo.wrapping_mode())
    {
    case core::pb::Texture_WrappingMode_REPEAT:
        wrappingMode = GL_REPEAT;
        break;
    case core::pb::Texture_WrappingMode_MIRROR_REPEAT:
        wrappingMode = GL_MIRRORED_REPEAT;
        break;
    case core::pb::Texture_WrappingMode_CLAMP_TO_EDGE:
        wrappingMode = GL_CLAMP_TO_EDGE;
        break;
    case core::pb::Texture_WrappingMode_CLAMP_TO_BORDER:
        wrappingMode = GL_CLAMP_TO_BORDER;
        break;
    default:
        break;
    }

    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrappingMode);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrappingMode);

    int minFilterMode = GL_NEAREST;
    int magFilterMode = GL_NEAREST;

    bool mipmap = textureInfo.generate_mipmaps();
    if(kTexture->numLevels > 1)
    {
        mipmap = true;
    }
    switch (textureInfo.filter_mode())
    {
    case core::pb::Texture_FilteringMode_LINEAR:
    {
        magFilterMode = GL_LINEAR;
        minFilterMode = mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        break;
    }
    case core::pb::Texture_FilteringMode_NEAREST:
    {
        magFilterMode = GL_NEAREST;
        minFilterMode = mipmap ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST;
        break;
    }
    default:
        break;
    }

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterMode);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterMode);

    return true;
}

bool Texture::LoadHdrTexture(const core::pb::Texture& textureInfo)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    stbi_set_flip_vertically_on_load(true);
    const auto& filesystem = core::FilesystemLocator::get();
    core::Path path{textureInfo.path()};
    if (filesystem.FileExists(path))
    {
#ifdef TRACY_ENABLE
        TracyCZoneN(ctx, "Load File", true);
#endif
        const auto file = filesystem.LoadFile(path);
#ifdef TRACY_ENABLE
        TracyCZoneEnd(ctx);

        TracyCZoneN(ctx2, "Decompress", true);
#endif
        int channelInFile;
        const auto* imageData = stbi_loadf_from_memory(file.data, file.size, &width, &height, &channelInFile, 0);
#ifdef TRACY_ENABLE
        TracyCZoneEnd(ctx2);
#endif
        if (imageData == nullptr)
        {
            LogError(fmt::format("Could not decode image from path: {}", path));
            return false;
        }
#ifdef TRACY_ENABLE
        ZoneNamedN(gpuUpload, "Upload to GPU", true);
#endif
        glGenTextures(1, &name);
        glCheckError();

        glBindTexture(GL_TEXTURE_2D, name);

        GLint wrappingMode = GL_REPEAT;
        switch (textureInfo.wrapping_mode())
        {
        case core::pb::Texture_WrappingMode_REPEAT:
            wrappingMode = GL_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_MIRROR_REPEAT:
            wrappingMode = GL_MIRRORED_REPEAT;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_EDGE:
            wrappingMode = GL_CLAMP_TO_EDGE;
            break;
        case core::pb::Texture_WrappingMode_CLAMP_TO_BORDER:
            wrappingMode = GL_CLAMP_TO_BORDER;
            break;
        default:
            break;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrappingMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrappingMode);

        int minFilterMode = GL_NEAREST;
        int magFilterMode = GL_NEAREST;
        switch (textureInfo.filter_mode())
        {
        case core::pb::Texture_FilteringMode_LINEAR:
        {
            magFilterMode = GL_LINEAR;
            minFilterMode = textureInfo.generate_mipmaps() ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            break;
        }
        case core::pb::Texture_FilteringMode_NEAREST:
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
#ifdef TRACY_ENABLE
        TracyGpuNamedZone(loadTexture, "Load Texture", true);
#endif
        switch (channelInFile)
        {
        case 1:
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height,
                0,
                GL_RED, GL_FLOAT,
                imageData);
            break;
        }
        case 2:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height,
                0,
                GL_RG, GL_FLOAT,
                imageData);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB16F, width, height,
                0,
                GL_RGB, GL_FLOAT,
                imageData);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
                0,
                GL_RGBA, GL_FLOAT,
                imageData);
            break;
        default:
            LogError(fmt::format("Invalid channel count on image. Count: {}, for texture at path: {}", channelInFile, path));
            return false;
        }
        glCheckError();
        if (textureInfo.generate_mipmaps())
        {
#ifdef TRACY_ENABLE
            TracyGpuNamedZone(generateMipMap, "Generate MipMap", true);
#endif
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        LogDebug(fmt::format("Successfully loaded texture at path: {}", path));
        return true;
    }
    LogError(fmt::format("File not found at path: {}", path));
    return false;
}

void Texture::Destroy()
{
    glDeleteTextures(1, &name);
    name = 0;
}
} // namespace gpr5300
