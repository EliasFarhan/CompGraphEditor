
#ifndef NEKO2_MATERIAL_H
#define NEKO2_MATERIAL_H

#include <generated/renderer_generated.h>


#include "buffer.h"
#include "pipeline.h"
#include "renderer/material.h"

namespace novus
{

class Material : public core::Material
{
public:
    explicit Material(Pipeline* pipeline);
    std::string_view GetName() const override;
    void Load(const renderer::MaterialT& materialInfo,
        const renderer::ShaderT& vertexShader,
        const renderer::ShaderT& fragmentShader,
        const BufferManager& bufferManager);
    int GetPipelineIndex() const{return pipelineIndex_;}
    void Bind(void* renderData);
private:
    struct StorageBufferBinding
    {
        Pipeline::StorageBufferId storageBufferId{};
        std::string bufferName;
    };
    std::vector<SDL_GPUBuffer*> vertexStorageBuffers;
    std::vector<SDL_GPUBuffer*> fragmentStorageBuffers;
    std::string name_;
    Pipeline* pipeline_ = nullptr;
    int pipelineIndex_ = -1;
};

} // namespace novus

#endif // NEKO2_MATERIAL_H
