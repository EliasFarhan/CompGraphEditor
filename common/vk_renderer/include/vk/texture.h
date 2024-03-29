#pragma once

#include "proto/renderer.pb.h"
#include "vk/common.h"
#include "renderer/texture.h"

namespace vk
{

class Texture : core::Texture
{
public:
    Texture() = default;
    ~Texture() override;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;
    bool LoadTexture(const core::pb::Texture& texture) override;
    bool LoadCubemap(const core::pb::Texture& texture) override;

    Image image;
    int width = 0;
    int height = 0;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    void Destroy();
private:
    bool CreateImageView(VkFormat format, int mipLevels, int layerCount);
    bool CreateSampler(const core::pb::Texture& texture);
};

class TextureManager : public core::TextureManager
{
public:
    core::TextureId LoadTexture(const core::pb::Texture &textureInfo) override;
    void Clear() override;
    const Texture& GetTexture(core::TextureId textureId) const;
private:
    std::unordered_map<std::string, core::TextureId> textureNamesMap_;
    std::vector<Texture> textures_;

};
}