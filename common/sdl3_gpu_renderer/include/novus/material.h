
#ifndef NEKO2_MATERIAL_H
#define NEKO2_MATERIAL_H

#include <generated/renderer_generated.h>


#include "buffer.h"
#include "framebuffer.h"
#include "pipeline.h"
#include "renderer/material.h"

namespace novus
{

class Material : public core::Material
{
public:
    explicit Material(Pipeline* pipeline);
    [[nodiscard]] std::string_view GetName() const override;
    void Load(const renderer::MaterialT& materialInfo,
        const BufferManager& bufferManager,
        std::span<core::TextureId> textures,
        std::span<Framebuffer> framebuffers);
    [[nodiscard]] int GetPipelineIndex() const{return pipelineIndex_;}
    void Bind(void* renderData);
    [[nodiscard]] std::span<const renderer::TextureMaterialBindingT> GetTextureBindings() const
    {
        return textureBindings;
    }
private:
    struct StorageBufferBinding
    {
        Pipeline::StorageBufferId storageBufferId{};
        std::string bufferName;
    };
    std::vector<renderer::TextureMaterialBindingT> textureBindings;
    std::vector<SDL_GPUTextureSamplerBinding> vertexTextures;
    std::vector<SDL_GPUTextureSamplerBinding> fragmentTextures;
    std::vector<SDL_GPUBuffer*> vertexStorageBuffers;
    std::vector<SDL_GPUBuffer*> fragmentStorageBuffers;
    std::string name_;
    Pipeline* pipeline_ = nullptr;
    int pipelineIndex_ = -1;
};

} // namespace novus

#endif // NEKO2_MATERIAL_H
