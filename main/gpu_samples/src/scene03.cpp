
#include "../../utils/shader_analyzer_lib/shader_analyzer.h"
#include "novus/mesh.h"
#include "sample.h"

namespace sample
{

novus::renderer::SceneT Scene03()
{
    novus::renderer::SceneT scene{};
    static constexpr std::string_view vertexPathBase = "data/shaders/03_hello_uniform/quad.vert";
    static constexpr std::string_view fragmentPathBase = "data/shaders/03_hello_uniform/quad.frag";

    scene.shaders.push_back({.path = (vertexPathBase.data()), .shader_stage = novus::internal::ShaderStage_VERTEX});
    const auto framgentAnalyzeResult = novus::GenerateShaderAttributeFromJson(std::string(fragmentPathBase)+".json");
    scene.shaders.push_back({.path = (fragmentPathBase.data()),
        .uniform_buffers = std::move(framgentAnalyzeResult.uniformBuffers),
        .types = std::move(framgentAnalyzeResult.types),
        .shader_stage = novus::internal::ShaderStage_FRAGMENT,});
    scene.name = "03_Uniform";

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
    novus::internal::ColorTargetDescriptionT colorTargetDescription{};
    //backbuffer format to get retrieve from the engine?
    colorTargetDescription.format = novus::internal::TextureFormat_TEXTUREFORMAT_B8G8R8A8_UNORM;
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
        .material_index = 0,
        .mesh_index = 0,
        .count = 6,
        .mode = novus::renderer::DrawMode_TRIANGLES,
        .draw_elements = true,
        .automatic_draw = false};

    novus::renderer::RenderpassT subpass{};
    subpass.name = "Main subpass";
    subpass.framebuffer_index = -1;
    subpass.commands.push_back(std::move(drawCommand));
    scene.sub_passes.push_back(subpass);

    scene.systems.push_back({.module_ = "scene03", .class_ = "Scene03", .path = "data/scripts/03_uniform.wasm"});

    return scene;
}
}