//
// Created by unite on 10.10.2025.
//

#include "novus/pipeline.h"

#include "novus/engine.h"
#include "utils/log.h"
#include <format>

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
        .stage = TranslateShaderState(shaderInfo.shader_stage),
        .num_samplers = (uint32_t)shaderInfo.samplers.size(),
        .num_storage_textures = shaderInfo.num_storage_textures,
        .num_storage_buffers = static_cast<uint32_t>(shaderInfo.storage_buffers.size()),
        .num_uniform_buffers = static_cast<uint32_t>(shaderInfo.uniform_buffers.size())
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
    //TODO needs to fill the other fields of stencil state
    auto* depth_stencil_state = pipelineInfo->depth_stencil_state.get();
    if (depth_stencil_state != nullptr)
    {
        //TODO needs to fill the other fields of stencil state
        pipelineCreateInfo.depth_stencil_state = {
            .compare_op = static_cast<SDL_GPUCompareOp>(depth_stencil_state->compare_op),
            .compare_mask = depth_stencil_state->compare_mask,
            .write_mask = depth_stencil_state->write_mask,
            .enable_depth_test = depth_stencil_state->enable_depth_test,
            .enable_depth_write = depth_stencil_state->enable_depth_write,
            .enable_stencil_test = depth_stencil_state->enable_stencil_test,
        };
    }

}
std::string AddFormatExtension(std::string_view glslPath)
{
    std::string newPath = glslPath.data();
    switch (core::GetShaderFormat())
    {
    case novus::engine::ShaderFormat_SPIRV:
        newPath += ".spv";
        break;
    case novus::engine::ShaderFormat_DXIL:
        newPath += ".dxil";
        break;
    case novus::engine::ShaderFormat_MSL:
        newPath += ".msl";
        break;
    case novus::engine::ShaderFormat_METALLIB:
        newPath += "metallib";
        break;
    case novus::engine::ShaderFormat_DXBC:
        newPath += ".cso";
        break;
    }
    return newPath;
}
void Pipeline::Load(const renderer::GraphicsPipelineT& pipelineInfo,
    const Shader& vertShader,
    const renderer::ShaderT& vertShaderInfo,
    const Shader& fragShader,
    const renderer::ShaderT& fragShaderInfo)
{
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.vertex_shader = vertShader.get();
    pipelineCreateInfo.fragment_shader = fragShader.get();

    FillPipelineInfo(pipelineCreateInfo, pipelineInfo.info.get());

    auto* vertexInputState = pipelineInfo.info->vertex_input_state.get();
    std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions;
    std::vector<SDL_GPUVertexAttribute> vertexBufferAttributes;
    if (vertexInputState != nullptr)
    {
        const auto& vertexDescriptions = vertexInputState->vertex_buffer_descriptions;
        const auto& vertexAttributes = vertexInputState->vertex_attributes;
        vertexBufferDescriptions.reserve(vertexDescriptions.size());
        vertexBufferAttributes.reserve(vertexAttributes.size());
        for(const auto& description : vertexDescriptions)
        {
            SDL_GPUVertexBufferDescription vertexBufferDescription{

                .slot = description.slot,
                .pitch = description.pitch,
                .input_rate = static_cast<SDL_GPUVertexInputRate>(description.input_rate),
                .instance_step_rate = description.instance_step_rate,
            };
            vertexBufferDescriptions.push_back(vertexBufferDescription);
        }

        for(const auto& attribute : vertexAttributes)
        {
            SDL_GPUVertexAttribute vertexBufferAttribute{
                .location = attribute.location,
                .buffer_slot = attribute.buffer_slot,
                .format = static_cast<SDL_GPUVertexElementFormat>(attribute.format),
                .offset = attribute.offset
            };
            vertexBufferAttributes.push_back(vertexBufferAttribute);
        }
        pipelineCreateInfo.vertex_input_state = {
            .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
            .num_vertex_buffers = static_cast<uint32_t>(vertexBufferDescriptions.size()),
            .vertex_attributes = vertexBufferAttributes.data(),
            .num_vertex_attributes = static_cast<uint32_t>(vertexBufferAttributes.size()) };
    }
    else
    {
        pipelineCreateInfo.vertex_input_state = { .num_vertex_buffers = 0, .num_vertex_attributes = 0 };
    }
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
    //Uniform data generation
    int currentUniformIndex = 0;
    for (const auto& vertexUniform : vertShaderInfo.uniform_buffers)
    {
        if (vertexUniform.block_size == 0)
            continue;

        UniformBuffer uniformBuffer{.data = std::make_unique<uint8_t[]>(vertexUniform.block_size),
            .binding = vertexUniform.binding,
            .stage = internal::ShaderStage_VERTEX,
            .block_size = (uint8_t)vertexUniform.block_size,
            .isDirty = false};
        uniformBuffers_.push_back(std::move(uniformBuffer));
        GenerateUniformBufferRef(vertexUniform.type_name, vertShaderInfo.types, currentUniformIndex);
        currentUniformIndex++;
    }
    for (const auto& vertexSsbo: vertShaderInfo.storage_buffers)
    {
        if (vertexSsbo.block_size == 0) continue;
        if (vertexSsbo.set != 0)
        {
            throw std::runtime_error("Invalid set for vertex storage buffer");
        }
        storageBuffersIds_.push_back({.stage = internal::ShaderStage_VERTEX, .binding = vertexSsbo.binding, .blockSize = vertexSsbo.block_size});
    }
    for (const auto& fragmentUniform : fragShaderInfo.uniform_buffers)
    {
        if (fragmentUniform.block_size == 0)
            continue;

        auto uniformBuffer = UniformBuffer{.data = std::make_unique<uint8_t[]>(fragmentUniform.block_size),
            .binding = fragmentUniform.binding,
            .stage = internal::ShaderStage_FRAGMENT,
            .block_size = (uint8_t)fragmentUniform.block_size,
            .isDirty = false};

        uniformBuffers_.push_back(std::move(uniformBuffer));
        GenerateUniformBufferRef(fragmentUniform.type_name, fragShaderInfo.types, currentUniformIndex);
        currentUniformIndex++;
    }

    for (const auto& fragSsbo: fragShaderInfo.storage_buffers)
    {
        if (fragSsbo.block_size == 0) continue;
        if (fragSsbo.set != 2)
        {
            throw std::runtime_error("Invalid set for fragment storage buffer");
        }
        storageBuffersIds_.push_back({.stage = internal::ShaderStage_FRAGMENT, .binding = fragSsbo.binding, .blockSize = fragSsbo.block_size});
    }
}
void Pipeline::Bind(void* renderData)
{
    //Needs current renderpass
    SDL_BindGPUGraphicsPipeline(static_cast<SDL_GPURenderPass*>(renderData), pipeline_);
}
void Pipeline::Destroy()
{
    if (pipeline_ != nullptr)
    {
        SDL_ReleaseGPUGraphicsPipeline(GetDevice(), pipeline_);
        pipeline_ = nullptr;
    }
}
void Pipeline::UploadDirtyUniformData()
{
    for (auto& buffer : uniformBuffers_)
    {
        if (buffer.isDirty)
        {
            switch (buffer.stage)
            {
            case internal::ShaderStage_VERTEX:
                SDL_PushGPUVertexUniformData(GetCommandBuffer(), buffer.binding, buffer.data.get(), buffer.block_size);
                buffer.isDirty = false;
                break;
            case internal::ShaderStage_FRAGMENT:
                SDL_PushGPUFragmentUniformData(GetCommandBuffer(), buffer.binding, buffer.data.get(),buffer.block_size);
                buffer.isDirty = false;
                break;
            default:
                break;
            }
        }
    }
}
void Pipeline::SetUniformData(std::string_view uniformName, const void* data, size_t length)
{
    auto it = uniformBufferReferenceMap_.find(uniformName.data());
    if (it == uniformBufferReferenceMap_.end())
    {
        throw std::runtime_error("Invalid uniform name");
    }
    //TODO test attribute type
    const auto& uniformRef = it->second;
    auto& uniformBuffer = uniformBuffers_[uniformRef.uniformIndex];
    std::memcpy(uniformBuffer.data.get()+uniformRef.offset, data, length);
    uniformBuffer.isDirty = true;
}
void Pipeline::GenerateUniformBufferRef(
    std::string_view currentTypeName,
    std::span<const internal::BufferStructT> types,
    int currentUniformIndex)
{
    auto typeIt = std::ranges::find_if(types, [&](const auto& type)
    {
        return currentTypeName == type.key;
    });
    if (typeIt == types.end())
    {
        throw std::runtime_error("Could not find the correct type in the shader");
    }
    for (auto& memberType : typeIt->members)
    {
        switch (auto type = core::GetAttributeType(memberType.type))
        {
        case internal::AttributeType_CUSTOM_STRUCT:
            GenerateUniformBufferRef(memberType.type, types, currentUniformIndex);
            break;
        case internal::AttributeType_FLOAT:
        case internal::AttributeType_VEC2:
        case internal::AttributeType_VEC3:
        case internal::AttributeType_VEC4:
        case internal::AttributeType_MAT2:
        case internal::AttributeType_MAT3:
        case internal::AttributeType_MAT4:
        case internal::AttributeType_INT:
        case internal::AttributeType_IVEC2:
        case internal::AttributeType_IVEC3:
        case internal::AttributeType_IVEC4:
        case internal::AttributeType_BOOL:
            uniformBufferReferenceMap_[memberType.name] = {.type = type,
            .uniformIndex = currentUniformIndex,
            .offset = memberType.offset};
            break;
        default:
            break;
        }
    }
}
} // namespace novus
