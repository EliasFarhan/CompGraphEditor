//
// Created by unite on 10.10.2025.
//

#include "novus/pipeline.h"

#include "novus/engine.h"
#include "utils/log.h"

namespace novus
{


constexpr SDL_GPUShaderStage TranslateShaderState(internal::ShaderStage stage)
{
    switch (stage)
    {
    case internal::ShaderStage_VERTEX:
        return SDL_GPU_SHADERSTAGE_VERTEX;
    case internal::ShaderStage_FRAGMENT:
        return SDL_GPU_SHADERSTAGE_FRAGMENT;
    default:
        break;
    }
    throw std::runtime_error("Invalid Shader Stage");
}

void Shader::LoadShader(const renderer::ShaderT& shaderInfo)
{
    size_t dataSize = 0;
    auto* shaderFile = SDL_LoadFile(shaderInfo.path.c_str(), &dataSize);
    if (shaderFile == nullptr)
    {
        throw std::runtime_error(std::format("Could not open {}", shaderInfo.path));
    }
    SDL_GPUShaderCreateInfo creationInfo{.code_size = dataSize,
        .code = static_cast<Uint8*>(shaderFile),
        .entrypoint = "main",
        .format = core::ConvertShaderFormat(core::GetShaderFormat()),
        .stage = TranslateShaderState(shaderInfo.info.shader_stage()),
        .num_samplers = shaderInfo.info.num_samplers(),
        .num_storage_textures = shaderInfo.info.num_storage_textures(),
        .num_storage_buffers = shaderInfo.info.num_storage_buffers(),
        .num_uniform_buffers = shaderInfo.info.num_uniform_buffers()
    };
    shader_ = SDL_CreateGPUShader(GetDevice(), &creationInfo);
    if (shader_ == nullptr)
    {
        throw std::runtime_error(std::format("Could not create shader {}", shaderInfo.path));
    }
}
void Shader::Destroy()
{
    if (shader_ != nullptr)
    {
        SDL_ReleaseGPUShader(GetDevice(), shader_);
        shader_ = nullptr;
    }
}
static void FillPipelineInfo(SDL_GPUGraphicsPipelineCreateInfo& pipelineCreateInfo, const internal::GraphicsPipelineInfoT* pipelineInfo)
{
    pipelineCreateInfo.primitive_type = static_cast<SDL_GPUPrimitiveType>(pipelineInfo->primitive_type);

    auto* rasterize_state = pipelineInfo->rasterizer_state.get();
    pipelineCreateInfo.rasterizer_state = {
        .fill_mode = static_cast<SDL_GPUFillMode>(rasterize_state->fill_mode),
        .cull_mode = static_cast<SDL_GPUCullMode>(rasterize_state->cull_mode),
        .front_face = static_cast<SDL_GPUFrontFace>(rasterize_state->front_face),
        .depth_bias_constant_factor = rasterize_state->depth_bias_constant_factor,
        .depth_bias_clamp = rasterize_state->depth_bias_clamp,
        .depth_bias_slope_factor = rasterize_state->depth_bias_slope_factor,
        .enable_depth_bias = rasterize_state->enable_depth_bias,
        .enable_depth_clip = rasterize_state->enable_depth_clip
    };
    auto* multisample_state = pipelineInfo->multisample_state.get();
    if (multisample_state != nullptr)
    {
        pipelineCreateInfo.multisample_state = {
            .sample_count = static_cast<SDL_GPUSampleCount>(multisample_state->sample_count),
            .sample_mask = multisample_state->sample_mask,
            .enable_mask = multisample_state->enable_mask,
        };
    }
    //TODO needs to fill the other fields of depth stencil state
    auto* depth_stencil_state = pipelineInfo->depth_stencil_state.get();
    if (depth_stencil_state != nullptr)
    {
        pipelineCreateInfo.depth_stencil_state = {
            .compare_op = static_cast<SDL_GPUCompareOp>(depth_stencil_state->compare_op),
            .write_mask = depth_stencil_state->write_mask,
            .enable_depth_test = depth_stencil_state->enable_depth_test,
            .enable_depth_write = depth_stencil_state->enable_depth_write,
            .enable_stencil_test = depth_stencil_state->enable_stencil_test,
        };
    }

}
void Pipeline::Load(const renderer::GraphicsPipelineT& pipelineInfo, const Shader& vertShader, const Shader& fragShader)
{
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertex_shader = vertShader.get();
    pipelineCreateInfo.fragment_shader = fragShader.get();

    FillPipelineInfo(pipelineCreateInfo, pipelineInfo.info.get());

    pipelineCreateInfo.vertex_input_state = { .num_vertex_buffers = 0, .num_vertex_attributes = 0 };

    auto* targetInfo = pipelineInfo.info->target_info.get();
    std::vector<SDL_GPUColorTargetDescription> color_target_descriptions;
    color_target_descriptions.reserve(targetInfo->color_target_descriptions.size());

    auto& targets = targetInfo->color_target_descriptions;
    for (const auto& color_target : targets) {
        SDL_GPUColorTargetDescription color_target_info{
            .format = static_cast<SDL_GPUTextureFormat>(color_target->format)
            //TODO add blend state
    };
        color_target_descriptions.push_back(color_target_info);
    }

    pipelineCreateInfo.target_info = {
        .color_target_descriptions = color_target_descriptions.data(),
        .num_color_targets = static_cast<Uint32>(color_target_descriptions.size()),
        .depth_stencil_format = static_cast<SDL_GPUTextureFormat>(targetInfo->depth_stencil_format),
        .has_depth_stencil_target = targetInfo->has_depth_stencil_target
    };
    pipeline_ = SDL_CreateGPUGraphicsPipeline(GetDevice(), &pipelineCreateInfo);
    if (!pipeline_) {
        throw std::runtime_error(std::format("Pipeline: Could not create: {}", SDL_GetError()));
    }
}
void Pipeline::Bind()
{
    //Needs current renderpass
    //SDL_BindGPUGraphicsPipeline(GetCurrentRenderpass(), pipeline_);
}
void Pipeline::Destroy()
{
    if (pipeline_ != nullptr)
    {
        SDL_ReleaseGPUGraphicsPipeline(GetDevice(), pipeline_);
        pipeline_ = nullptr;
    }
}
} // namespace novus
