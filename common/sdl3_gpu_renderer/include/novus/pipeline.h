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
    SDL_GPUShader* get() const { return shader_;}
private:
    SDL_GPUShader* shader_ = nullptr;
};

class Pipeline : public core::Pipeline
{
public:
    void Load(const renderer::GraphicsPipelineT& pipelineInfo, const Shader& vertShader, const Shader& fragShader);
	void Bind() override;
    void Destroy();
    auto* get(){return pipeline_;}
private:
    SDL_GPUGraphicsPipeline* pipeline_ = nullptr;
};
}
#endif //NEKO2_PIPELINE_H
