//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_PIPELINE_H
#define NEKO2_PIPELINE_H
#include <SDL3/SDL_gpu.h>


#include "novus/engine.h"
#include "renderer/pipeline.h"
#include "generated/engine_generated.h"

namespace novus
{


class Shader : core::Shader
{
public:
    void LoadShader(const renderer::ShaderT& shaderInfo);
    void Destroy();
    [[nodiscard]] SDL_GPUShader* get() const { return shader_;}
private:
    SDL_GPUShader* shader_ = nullptr;
};

std::string AddFormatExtension(std::string_view glslPath);


class Pipeline : public core::Pipeline
{
public:
    void Load(const renderer::GraphicsPipelineT& pipelineInfo,
        const Shader& vertShader,
        const renderer::ShaderT& vertShaderInfo,
        const Shader& fragShader,
        const renderer::ShaderT& fragShaderInfo);
	void Bind(void* renderData) override;
    void Destroy();
private:
    struct UniformBuffer
    {
        std::unique_ptr<uint8_t[]> data;
        int binding{};
        internal::ShaderStage stage = (internal::ShaderStage)-1;
        bool isDirty{};
    };

    struct UniformBufferReference
    {
        internal::AttributeType type;
        int uniformIndex;
        int offset;
    };
    void GenerateUniformBufferRef(std::string_view currentTypeName,
        std::span<const internal::BufferStructT> types,
        int currentUniformIndex);
    SDL_GPUGraphicsPipeline* pipeline_ = nullptr;
    std::vector<UniformBuffer> uniformBuffers_;

    std::unordered_map<std::string, UniformBufferReference> uniformBufferReferenceMap_{};
};
}
#endif //NEKO2_PIPELINE_H
