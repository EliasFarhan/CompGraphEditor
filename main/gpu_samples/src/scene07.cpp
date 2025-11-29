#include "novus/mesh.h"
#include "sample.h"
#include "shader_analyzer.h"
namespace sample
{
novus::renderer::SceneT Scene07()
{
    novus::renderer::SceneT scene{};



    static constexpr std::string_view vertexPathBase = "data/shaders/07_hello_post_process/model.vert";
    static constexpr std::string_view fragmentPathBase = "data/shaders/07_hello_post_process/model.frag";
    static constexpr std::string_view postVertexPathBase = "data/shaders/07_hello_post_process/screen.vert";
    static constexpr std::string_view postFragmentPathBase = "data/shaders/07_hello_post_process/screen_edge_detection.frag";
    const auto vertexAnalyzeResult = novus::GenerateShaderAttributeFromJson(std::string(vertexPathBase)+".json");
    const auto fragmentAnalyzeResult = novus::GenerateShaderAttributeFromJson(std::string(fragmentPathBase)+".json");

    scene.shaders.push_back({.path = novus::AddFormatExtension(vertexPathBase),
        .types = vertexAnalyzeResult.types,
        .storage_buffers = vertexAnalyzeResult.storageBuffers,
        .shader_stage = novus::internal::ShaderStage_VERTEX});
    scene.shaders.push_back({.path = novus::AddFormatExtension(fragmentPathBase),
        .samplers = fragmentAnalyzeResult.shaderSamplers,
        .shader_stage = novus::internal::ShaderStage_FRAGMENT,
        });
    scene.shaders.push_back({.path = novus::AddFormatExtension(postVertexPathBase),
        .shader_stage = novus::internal::ShaderStage_VERTEX});
    const auto postFragmentAnaylzeResult = novus::GenerateShaderAttributeFromJson(std::string(postFragmentPathBase)+".json");
    scene.shaders.push_back({.path = novus::AddFormatExtension(postFragmentPathBase),
        .samplers = postFragmentAnaylzeResult.shaderSamplers,
        .shader_stage = novus::internal::ShaderStage_FRAGMENT,
        });
    novus::renderer::FramebufferT framebuffer{};
    static constexpr auto renderColorTargetFormat = novus::internal::TextureFormat_TEXTUREFORMAT_R8G8B8A8_UNORM;
    static constexpr auto renderDepthTargetFormat = novus::internal::TextureFormat_TEXTUREFORMAT_D24_UNORM_S8_UINT;
    novus::renderer::ColorAttachmentT colorAttachment{};

    auto colorTextureInfo = std::make_unique<novus::internal::TextureInfoT>();
    *colorTextureInfo = novus::internal::TextureInfoT{.width = 0, .height = 0,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .format = renderColorTargetFormat,
        .sample_count = novus::internal::SampleCount_SAMPLECOUNT_1,
        .type = novus::internal::TextureType_TEXTURETYPE_2D,
        .usage = (novus::internal::TextureUsageFlags)(novus::internal::TextureUsageFlags_TEXTUREUSAGE_SAMPLER | novus::internal::TextureUsageFlags_TEXTUREUSAGE_COLOR_TARGET)};
    colorAttachment.texture_info = std::move(colorTextureInfo);
    auto colorTargetInfo = std::make_unique<novus::internal::ColorTargetInfoT>();
    *colorTargetInfo = novus::internal::ColorTargetInfoT{.mip_level = 0,
        .layer_or_depth_plane = 0,
        .clear_color = {0,0,0,0},
        .load_op = novus::internal::LoadOp_LOADOP_CLEAR, .store_op = novus::internal::StoreOp_STOREOP_STORE};
    colorAttachment.target_info = std::move(colorTargetInfo);
    auto colorSamplerInfo = std::make_unique<novus::internal::SamplerInfoT>();
    colorSamplerInfo->address_mode_u = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    colorSamplerInfo->address_mode_v = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    colorSamplerInfo->address_mode_w = novus::internal::SamplerAddressMode_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    colorSamplerInfo->min_filter = novus::internal::FilterMode_FILTER_LINEAR;
    colorSamplerInfo->mag_filter = novus::internal::FilterMode_FILTER_LINEAR;
    colorSamplerInfo->compare_op = novus::internal::CompareOp_COMPAREOP_ALWAYS;
    colorSamplerInfo->mipmap_mode = novus::internal::SamplerMipmapMode_SAMPLERMIPMAPMODE_LINEAR;
    colorSamplerInfo->max_anisotropy = 1.0f;
    colorSamplerInfo->max_lod = 0.0f;
    colorSamplerInfo->min_lod = 0.0f;
    colorSamplerInfo->mip_lod_bias = 0.0f;
    colorSamplerInfo->enable_anisotropy = true;
    colorSamplerInfo->enable_compare = false;
    colorAttachment.sampler_info = (std::move(colorSamplerInfo));
    framebuffer.color_attachments.push_back(std::move(colorAttachment));

    auto depthStencilAttachment = std::make_unique<novus::renderer::DepthStencilAttachmentT>();
    auto depthTextureInfo = std::make_unique<novus::internal::TextureInfoT>();
    depthTextureInfo->width = 0;
    depthTextureInfo->height = 0;
    depthTextureInfo->format = renderDepthTargetFormat;
    depthTextureInfo->layer_count_or_depth = 1;
    depthTextureInfo->num_levels = 1;
    depthTextureInfo->sample_count = novus::internal::SampleCount_SAMPLECOUNT_1;
    depthTextureInfo->type = novus::internal::TextureType_TEXTURETYPE_2D;
    depthTextureInfo->usage = novus::internal::TextureUsageFlags_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    depthStencilAttachment->texture_info = std::move(depthTextureInfo);
    auto depthStencilTargetInfo = std::make_unique<novus::internal::DepthStencilTargetInfoT>();
    depthStencilTargetInfo->clear_depth = 1.0f;
    depthStencilTargetInfo->load_op = novus::internal::LoadOp_LOADOP_CLEAR;
    depthStencilTargetInfo->store_op = novus::internal::StoreOp_STOREOP_STORE;
    depthStencilAttachment->target_info = std::move(depthStencilTargetInfo);

    framebuffer.depth_stencil_attachment = std::move(depthStencilAttachment);

    scene.framebuffers.push_back(std::move(framebuffer));
    scene.name = "07_Post-Process";
    {
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
        //Settings from the frambuffer, maybe cache it first?
        colorTargetDescription->format = renderColorTargetFormat;
        targetInfo->color_target_descriptions.push_back(std::move(colorTargetDescription));
        targetInfo->has_depth_stencil_target = true;
        targetInfo->depth_stencil_format = renderDepthTargetFormat;
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
    }
    {
        novus::renderer::GraphicsPipelineT postGraphicsPipeline;
        postGraphicsPipeline.vertex_shader_index = 2;
        postGraphicsPipeline.fragment_shader_index = 3;

        auto postGraphicsPipelineInfo = std::make_unique<novus::internal::GraphicsPipelineInfoT>();
        postGraphicsPipelineInfo->primitive_type = novus::internal::PrimitiveType_TRIANGLELIST;
        postGraphicsPipelineInfo->vertex_input_state = novus::GenerateVertexInputState();

        auto postRasterizerState = std::make_unique<novus::internal::RasterizerStateT>();
        postRasterizerState->cull_mode = novus::internal::CullMode_NONE;
        postRasterizerState->fill_mode = novus::internal::FillMode_FILL;
        postGraphicsPipelineInfo->rasterizer_state = std::move(postRasterizerState);

        auto postTargetInfo = std::make_unique<novus::internal::GraphicsPipelineTargetInfoT>();
        auto postColorTargetDescription = std::make_unique<novus::internal::ColorTargetDescriptionT>();
        //Settings from the frambuffer, maybe cache it first?
        postColorTargetDescription->format = (novus::internal::TextureFormat)novus::GetSwapchainTextureFormat();
        postTargetInfo->color_target_descriptions.push_back(std::move(postColorTargetDescription));
        postTargetInfo->has_depth_stencil_target = false;
        postGraphicsPipelineInfo->target_info = std::move(postTargetInfo);

        postGraphicsPipeline.info = std::move(postGraphicsPipelineInfo);
        scene.pipelines.push_back(std::move(postGraphicsPipeline));
    }
    {
        std::vector<novus::renderer::StorageBufferBindingT> storageBufferBindings;
        storageBufferBindings.push_back({.buffer_name = "ubo",
            .binding = 0,
            .shader_stage = novus::internal::ShaderStage_VERTEX});
        std::vector<novus::renderer::TextureMaterialBindingT> textureBindings;
        textureBindings.push_back({.binding = 0, .set = 2, .texture_index = 0});
        novus::renderer::MaterialT material{.name = "Cube Material",
            .pipeline_index = 0,
            .storage_buffer_bindings = std::move(storageBufferBindings),
            .texture_bindings = std::move(textureBindings)};
        scene.materials.push_back(std::move(material));
    }
    {
        novus::renderer::MaterialT postMaterial{.name = "Screen Material",
        .pipeline_index = 1};
        postMaterial.texture_bindings.push_back(
            {.binding = 0,
                .set = 2,
                .texture_index = 0,
                .framebuffer_index = 0});
        scene.materials.push_back(std::move(postMaterial));
    }
    novus::renderer::MeshT cube{.mesh_name = "Cube", .model_index = -1, .primitive_type = novus::renderer::MeshPrimitiveType_CUBE};
    scene.meshes.push_back(std::move(cube));
    novus::renderer::MeshT screenQuad{.mesh_name = "Screen Quad", .model_index = -1, .primitive_type = novus::renderer::MeshPrimitiveType_QUAD, .scale = glm::vec3(2.0f)};
    scene.meshes.emplace_back(std::move(screenQuad));

    novus::renderer::DrawCommandT drawCommand{.name = "Draw Cube",
        .material_index = 0,
        .mesh_index = 0,
        .count = 36,
        .mode = novus::renderer::DrawMode_TRIANGLES,
        .draw_elements = true,
        .automatic_draw = true};

    novus::renderer::RenderpassT subpass{};
    subpass.name = "Main subpass";
    subpass.framebuffer_index = 0;
    subpass.commands.push_back(std::move(drawCommand));
    scene.sub_passes.push_back(subpass);

    novus::renderer::DrawCommandT postDrawCommand{.name = "Draw Post Process",
        .material_index = 1,
        .mesh_index = 1,
        .count = 6,
        .mode = novus::renderer::DrawMode_TRIANGLES,
        .draw_elements = true,
        .automatic_draw = true};

    novus::renderer::RenderpassT postSubpass{};
    postSubpass.name = "Post Process subpass";
    postSubpass.framebuffer_index = -1;
    postSubpass.commands.push_back(std::move(postDrawCommand));
    scene.sub_passes.push_back(postSubpass);

    scene.systems.push_back({.module_ = "scene04", .class_ = "Scene04", .path = "data/scripts/04_storage.wasm"});

    scene.buffers.push_back({.name = "ubo",
        .block_size = 3*sizeof(glm::mat4)});
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

    return scene;
}
}