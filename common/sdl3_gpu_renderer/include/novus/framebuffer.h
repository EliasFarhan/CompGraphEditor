//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_FRAMEBUFFER_H
#define NEKO2_FRAMEBUFFER_H
#include <SDL3/SDL_gpu.h>


#include "renderer/framebuffer.h"
#include "texture.h"

namespace novus
{
class Framebuffer : public core::Framebuffer
{
public:
	std::unique_ptr<core::Image> GetImage(std::string_view attachmentName) override;
    void Load(const renderer::FramebufferT& framebufferInfo) override;
    [[nodiscard]] std::span<const SDL_GPUColorTargetInfo> GetColorTargets() const{return colorTargets_;}

    [[nodiscard]] std::span<Texture> GetColorTextures(){return colorTextures_;}
    [[nodiscard]] const SDL_GPUDepthStencilTargetInfo* GetDepthStencilTargetInfo() const
    {
        return depthTexture_ == nullptr ? nullptr : &depthStencilTargetInfo_;
    }

    void UpdateColorTargetTexture(SDL_GPUTexture* sdl_gpu_texture);

    void Clear();

private:
    std::vector<Texture> colorTextures_;
    std::vector<SDL_GPUColorTargetInfo> colorTargets_;
    SDL_GPUTexture* depthTexture_ = nullptr;
    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo_{};
};

}
#endif //NEKO2_FRAMEBUFFER_H
