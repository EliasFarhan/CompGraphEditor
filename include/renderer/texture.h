#pragma once

#include <GL/glew.h>
#include "proto/renderer.pb.h"

namespace gpr5300
{
    //A loaded image in memory
    class Image
    {

    };

    //A loaded texture in GPU that can be used in OpenGL
    struct Texture
    {
        GLuint name = 0;
        int width = 0;
        int height = 0;
        GLenum target = GL_TEXTURE_2D;
        bool LoadTexture(const pb::Texture& textureInfo);
        bool LoadCubemap(const pb::Texture& textureInfo);
        bool LoadKtxTexture(const pb::Texture& textureInfo);
        void Destroy();
    };

    class TextureManager
    {
    public:
        Texture LoadTexture(const pb::Texture& textureInfo);
    private:
        std::unordered_map<std::string, Texture> texturesMap_;
    };
} // namespace gpr5300
