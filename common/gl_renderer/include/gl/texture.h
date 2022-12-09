#pragma once

#include "renderer/texture.h"

#include <GL/glew.h>
#include "proto/renderer.pb.h"

namespace gpr5300::gl
{

    //A loaded texture in GPU that can be used in OpenGL
    class Texture : gpr5300::Texture
    {
    public:
        Texture() = default;
        ~Texture() override;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) noexcept = default;
        Texture& operator=(Texture&&) noexcept = default;

        GLuint name = 0;
        int width = 0;
        int height = 0;
        GLenum target = GL_TEXTURE_2D;

        bool LoadTexture(const pb::Texture& textureInfo) override;
        bool LoadCubemap(const pb::Texture& textureInfo) override;
        bool LoadKtxTexture(const pb::Texture& textureInfo);
        void Destroy();
    };

class TextureManager : public gpr5300::TextureManager
{
public:
    TextureId LoadTexture(const pb::Texture& textureInfo) override;
    const Texture& GetTexture(TextureId textureId);
private:
    std::unordered_map<std::string, TextureId> textureNamesMap_;
    std::vector<Texture> textures_;
};
} // namespace gpr5300
