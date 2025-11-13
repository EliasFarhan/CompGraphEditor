//
// Created by unite on 10.10.2025.
//

#ifndef NEKO2_PIPELINE_H
#define NEKO2_PIPELINE_H


#include <SDL3/SDL_gpu.h>

#include "novus/engine.h"
#include "renderer/pipeline.h"
#include "generated/engine_generated.h"
#include <memory>

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
    Pipeline() = default;
    Pipeline(Pipeline const&) = delete;
    Pipeline& operator=(Pipeline const&) = delete;
    Pipeline(Pipeline&& other) noexcept:
        pipeline_(other.pipeline_),
        uniformBuffers_(std::move(other.uniformBuffers_)),
        uniformBufferReferenceMap_(std::move(other.uniformBufferReferenceMap_))
    {

    }
    Pipeline& operator=(Pipeline&& other) noexcept
    {
        std::swap(pipeline_, other.pipeline_);
        std::swap(uniformBuffers_, other.uniformBuffers_);
        std::swap(uniformBufferReferenceMap_, other.uniformBufferReferenceMap_);
        return *this;
    }

    void Load(const renderer::GraphicsPipelineT& pipelineInfo,
        const Shader& vertShader,
        const renderer::ShaderT& vertShaderInfo,
        const Shader& fragShader,
        const renderer::ShaderT& fragShaderInfo);
	void Bind(void* renderData) override;
    void Destroy();
    void UploadDirtyUniformData() override;
protected:
    void SetUniformData(std::string_view uniformName, const void* data, size_t length) override;
private:
    struct UniformBuffer
    {
        std::unique_ptr<uint8_t[]> data;
        int binding{};
        internal::ShaderStage stage = (internal::ShaderStage)-1;
        uint8_t block_size;
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
