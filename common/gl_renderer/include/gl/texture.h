#pragma once

#include "renderer/texture.h"

#include <GL/glew.h>
#include "proto/renderer.pb.h"

namespace gpr5300::gl
{
    //A loaded image in memory
    class Image
    {

    };

    //A loaded texture in GPU that can be used in OpenGL
    struct Texture
    {
        ~Texture();
        GLuint name = 0;
        int width = 0;
        int height = 0;
        GLenum target = GL_TEXTURE_2D;
        bool LoadTexture(const pb::Texture& textureInfo);
        bool LoadCubemap(const pb::Texture& textureInfo);
        bool LoadKtxTexture(const pb::Texture& textureInfo);
        void Destroy();
    };

class TextureManager : public gpr5300::TextureManager
{
public:
    Texture LoadTexture(const pb::Texture& textureInfo) override;
private:
    std::unordered_map<std::string, Texture> texturesMap_;
};
} // namespace gpr5300
