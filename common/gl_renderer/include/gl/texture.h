#pragma once

#include "renderer/texture.h"

#include <GL/glew.h>
#include "proto/renderer.pb.h"

namespace gl
{

//A loaded texture in GPU that can be used in OpenGL
class Texture : core::Texture
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

    bool LoadTexture(const core::pb::Texture& textureInfo) override;
    bool LoadCubemap(const core::pb::Texture& textureInfo) override;
    bool LoadKtxTexture(const core::pb::Texture& textureInfo);
    void Destroy();
};

class TextureManager : public core::TextureManager
{
public:
    core::TextureId LoadTexture(const core::pb::Texture& textureInfo) override;
    const Texture& GetTexture(core::TextureId textureId);
    void Clear() override;
private:
    std::unordered_map<std::string, core::TextureId> textureNamesMap_;
    std::vector<Texture> textures_;
};
} // namespace gpr5300
