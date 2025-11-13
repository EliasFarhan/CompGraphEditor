//
// Created by unite on 06.11.2025.
//
#include "novus/mesh.h"
#include "sample.h"

namespace sample
{

novus::renderer::SceneT Scene02()
{
    novus::renderer::SceneT scene{};
    static constexpr std::string_view vertexPathBase = "data/shaders/02_hello_quad/quad.vert";
    static constexpr std::string_view fragmentPathBase = "data/shaders/02_hello_quad/quad.frag";

    scene.shaders.push_back({.path = novus::AddFormatExtension(vertexPathBase), .shader_stage = novus::internal::ShaderStage_VERTEX});
    scene.shaders.push_back({.path = novus::AddFormatExtension(fragmentPathBase), .shader_stage = novus::internal::ShaderStage_FRAGMENT});
    scene.name = "02_Quad";

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
    colorTargetDescription->format = novus::internal::TextureFormat_TEXTUREFORMAT_B8G8R8A8_UNORM;
    targetInfo->color_target_descriptions.push_back(std::move(colorTargetDescription));
    targetInfo->has_depth_stencil_target = false;
    graphicsPipelineInfo->target_info = std::move(targetInfo);

    graphicsPipeline.info = std::move(graphicsPipelineInfo);
    scene.pipelines.push_back(std::move(graphicsPipeline));

    novus::renderer::MaterialT material{.name = "Quad Material", .pipeline_index = 0};
    scene.materials.push_back(std::move(material));

    novus::renderer::MeshT mesh{.mesh_name = "Quad", .model_index = -1, .primitive_type = novus::renderer::MeshPrimitiveType_QUAD};
    scene.meshes.push_back(std::move(mesh));

    novus::renderer::DrawCommandT drawCommand{.name = "Draw Quad",
        .material_index = 0, .mesh_index = 0, .count = 6,
        .mode = novus::renderer::DrawMode_TRIANGLES, .draw_elements = true, .automatic_draw = true};

    novus::renderer::RenderpassT subpass{};
    subpass.name = "Main subpass";

    auto renderPassInfo = std::make_unique<novus::internal::RenderPassInfoT>();
    renderPassInfo->color_target_infos.push_back({.mip_level = 0, .layer_or_depth_plane = 0,
        .clear_color = {0.0f,0.0f,0.0f,0.0f},
        .load_op = novus::internal::LoadOp_LOADOP_CLEAR, .store_op = novus::internal::StoreOp_STOREOP_STORE});
    subpass.info = std::move(renderPassInfo);
    subpass.commands.push_back(std::move(drawCommand));
    scene.sub_passes.push_back(subpass);

    return scene;
}
}