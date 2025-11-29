#include "novus/mesh.h"
#include "sample.h"
#include "shader_analyzer.h"

namespace sample
{
novus::renderer::SceneT Scene06()
{
    novus::renderer::SceneT scene{};
    static constexpr std::string_view vertexPathBase = "data/shaders/06_hello_cube/cube.vert";
    static constexpr std::string_view fragmentPathBase = "data/shaders/06_hello_cube/cube.frag";
    const auto vertexAnalyzeResult = novus::GenerateShaderAttributeFromJson(std::string(vertexPathBase)+".json");
    const auto fragmentAnalyzeResult = novus::GenerateShaderAttributeFromJson(std::string(fragmentPathBase)+".json");

    scene.shaders.push_back({.path = (vertexPathBase.data()),
        .types = vertexAnalyzeResult.types,
        .storage_buffers = vertexAnalyzeResult.storageBuffers,
        .shader_stage = novus::internal::ShaderStage_VERTEX});
    scene.shaders.push_back({.path = (fragmentPathBase.data()),
        .samplers = fragmentAnalyzeResult.shaderSamplers,
        .shader_stage = novus::internal::ShaderStage_FRAGMENT});
    scene.name = "06_Cube";

    novus::renderer::GraphicsPipelineT graphicsPipeline{};
    graphicsPipeline.vertex_shader_index = 0;
    graphicsPipeline.fragment_shader_index = 1;

    auto graphicsPipelineInfo = std::make_unique<novus::internal::GraphicsPipelineInfoT>();
    graphicsPipelineInfo->primitive_type = novus::internal::PrimitiveType_TRIANGLELIST;

    //this is know by the engine, no?
    graphicsPipelineInfo->vertex_input_state = novus::GenerateVertexInputState();
    auto rasterizerState = std::make_unique<novus::internal::RasterizerStateT>();
    rasterizerState->cull_mode = novus::internal::CullMode_NONE;
    rasterizerState->fill_mode = novus::internal::FillMode_FILL;
    graphicsPipelineInfo->rasterizer_state = std::move(rasterizerState);

    auto targetInfo = std::make_unique<novus::internal::GraphicsPipelineTargetInfoT>();
    auto colorTargetDescription = std::make_unique<novus::internal::ColorTargetDescriptionT>();
    //backbuffer format to get retrieve from the engine?
    colorTargetDescription->format = (novus::internal::TextureFormat)novus::GetSwapchainTextureFormat();
    targetInfo->color_target_descriptions.push_back(std::move(colorTargetDescription));
    targetInfo->has_depth_stencil_target = true;
    targetInfo->depth_stencil_format = novus::internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    graphicsPipelineInfo->target_info = std::move(targetInfo);

    auto depthStencilState = std::make_unique<novus::internal::DepthStencilStateT>();
    depthStencilState->enable_depth_test = true;
    depthStencilState->enable_depth_write = true;
    depthStencilState->compare_op = novus::internal::CompareOp_COMPAREOP_LESS;
    depthStencilState->write_mask = 0xFF;
    depthStencilState->compare_mask = 0xFF;
    graphicsPipelineInfo->depth_stencil_state = std::move(depthStencilState);

    graphicsPipeline.info = std::move(graphicsPipelineInfo);
    scene.pipelines.push_back(std::move(graphicsPipeline));

    std::vector<novus::renderer::StorageBufferBindingT> storageBufferBindings;
    storageBufferBindings.push_back({.buffer_name = "camera", .binding = 0, .shader_stage = novus::internal::ShaderStage_VERTEX});
    storageBufferBindings.push_back({.buffer_name = "transform", .binding = 1, .shader_stage = novus::internal::ShaderStage_VERTEX});
    std::vector<novus::renderer::TextureMaterialBindingT> textureBindings;
    textureBindings.push_back({.binding = 0, .set = 2, .texture_index = 0, .framebuffer_index = -1});
    novus::renderer::MaterialT material{.name = "Cube Material",
        .pipeline_index = 0, .storage_buffer_bindings = std::move(storageBufferBindings), .texture_bindings = std::move(textureBindings)};
    scene.materials.push_back(std::move(material));

    novus::renderer::MeshT mesh{.mesh_name = "Cube", .model_index = -1, .primitive_type = novus::renderer::MeshPrimitiveType_CUBE};
    scene.meshes.push_back(std::move(mesh));

    novus::renderer::DrawCommandT drawCommand{.name = "Draw Cube",
        .material_index = 0,
        .mesh_index = 0,
        .count = 36,
        .mode = novus::renderer::DrawMode_TRIANGLES,
        .draw_elements = true,
        .automatic_draw = false};

    novus::renderer::RenderpassT subpass{};
    subpass.name = "Main subpass";

    subpass.framebuffer_index = -1;
    subpass.commands.push_back(std::move(drawCommand));
    scene.sub_passes.push_back(subpass);

    novus::renderer::TextureT texture;
    auto samplerInfo = std::make_unique<novus::internal::SamplerInfoT>();
    samplerInfo->address_mode_u = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo->address_mode_v = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo->address_mode_w = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo->min_filter = novus::internal::FilterMode_FILTER_LINEAR;
    samplerInfo->mag_filter = novus::internal::FilterMode_FILTER_LINEAR;
    samplerInfo->compare_op = novus::internal::CompareOp_COMPAREOP_ALWAYS;
    samplerInfo->mipmap_mode = novus::internal::SamplerMipmapMode_SAMPLERMIPMAPMODE_LINEAR;
    samplerInfo->max_anisotropy = 1.0f;
    samplerInfo->max_lod = 0.0f;
    samplerInfo->min_lod = 0.0f;
    samplerInfo->mip_lod_bias = 0.0f;
    samplerInfo->enable_anisotropy = true;
    samplerInfo->enable_compare = false;

    texture.sampler = std::move(samplerInfo);

    auto textureInfo = std::make_unique<novus::internal::TextureInfoT>();
    textureInfo->format = novus::internal::TextureFormat_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo->width = 500;
    textureInfo->height = 500;
    textureInfo->layer_count_or_depth = 1;
    textureInfo->num_levels = 1;
    textureInfo->sample_count = novus::internal::SampleCount_SAMPLECOUNT_1;
    textureInfo->type = novus::internal::TextureType_TEXTURETYPE_2D;
    textureInfo->usage = novus::internal::TextureUsageFlags_TEXTUREUSAGE_SAMPLER;

    texture.info = std::move(textureInfo);
    texture.path = "data/textures/container2.png";

    scene.textures.push_back(std::move(texture));

    scene.systems.push_back({.module_ = "scene06", .class_ = "Scene06", .path = "data/scripts/06_instanced.wasm"});

    scene.buffers.push_back({.name = "camera", .block_size = 2*sizeof(glm::mat4)});
    scene.buffers.push_back({.name = "transform", .block_size = 9*sizeof(glm::mat4)});
    return scene;
}
}