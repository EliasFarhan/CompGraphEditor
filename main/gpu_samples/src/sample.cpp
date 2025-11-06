#include "sample.h"

#include <generated/renderer_generated.h>
#include <imgui.h>

#include "novus/scene.h"

namespace sample
{




novus::renderer::SceneT Scene01()
{
    novus::renderer::SceneT scene{};
    novus::internal::ShaderInfo vertexShaderInfo(0,0,0,0,novus::internal::ShaderStage_VERTEX);
    novus::internal::ShaderInfo fragmentShaderInfo(0,0,0,0,novus::internal::ShaderStage_FRAGMENT);
    static constexpr std::string_view vertexPathBase = "data/shaders/01_hello_triangle/triangle.vert";
    static constexpr std::string_view fragmentPathBase = "data/shaders/01_hello_triangle/triangle.frag";

    std::string vertexPath = vertexPathBase.data();
    std::string fragmentPath = fragmentPathBase.data();
    switch (core::GetShaderFormat())
    {

    case novus::engine::ShaderFormat_SPIRV:
        vertexPath += ".spv";
        fragmentPath += ".spv";
        break;
    case novus::engine::ShaderFormat_DXIL:
        break;
    case novus::engine::ShaderFormat_MSL:
        break;
    case novus::engine::ShaderFormat_METALLIB:
        break;
    case novus::engine::ShaderFormat_DXBC:
        break;
    }
    scene.shaders.push_back({.path = vertexPath, .info = vertexShaderInfo});
    scene.shaders.push_back({.path = fragmentPath, .info = fragmentShaderInfo});
    scene.name = "01_Triangle";

    novus::renderer::GraphicsPipelineT graphicsPipeline{};
    graphicsPipeline.vertex_shader_index = 0;
    graphicsPipeline.fragment_shader_index = 1;

    auto graphicsPipelineInfo = std::make_unique<novus::internal::GraphicsPipelineInfoT>();
    graphicsPipelineInfo->primitive_type = novus::internal::PrimitiveType_TRIANGLELIST;

    auto rasterizerState = std::make_unique<novus::internal::RasterizerStateT>();
    rasterizerState->cull_mode = novus::internal::CullMode_NONE;
    rasterizerState->fill_mode = novus::internal::FillMode_FILL;
    graphicsPipelineInfo->rasterizer_state = std::move(rasterizerState);

    auto targetInfo = std::make_unique<novus::internal::GraphicsPipelineTargetInfoT>();
    auto colorTargetDescription = std::make_unique<novus::internal::ColorTargetDescriptionT>();
    //backbuffer format
    colorTargetDescription->format = novus::internal::TextureFormat_TEXTUREFORMAT_B8G8R8A8_UNORM;
    targetInfo->color_target_descriptions.push_back(std::move(colorTargetDescription));
    targetInfo->has_depth_stencil_target = false;
    graphicsPipelineInfo->target_info = std::move(targetInfo);



    graphicsPipeline.info = std::move(graphicsPipelineInfo);
    scene.pipelines.push_back(std::move(graphicsPipeline));

    novus::renderer::MaterialT material{.name = "Triangle Material", .pipeline_index = 0};
    scene.materials.push_back(std::move(material));

    novus::renderer::MeshT mesh{.mesh_name = "Triangle", .model_index = -1, .primitive_type = novus::renderer::MeshPrimitiveType_NONE};
    scene.meshes.push_back(std::move(mesh));

    novus::renderer::DrawCommandT drawCommand{.name = "Draw Triangle",
        .material_index = 0, .mesh_index = 0, .count = 3,
        .mode = novus::renderer::DrawMode_TRIANGLES, .draw_elements = false, .automatic_draw = true};

    novus::renderer::SubpassT subpass{};
    subpass.name = "Main subpass";

    auto renderPassInfo = std::make_unique<novus::internal::RenderPassInfoT>();
    renderPassInfo->color_target_infos.push_back({.mip_level = 0, .layer_or_depth_plane = 0,
        .clear_color = {0.0f,0.0f,0.0f,0.0f},
        .load_op = novus::internal::LoadOp_LOADOP_CLEAR, .store_op = novus::internal::StoreOp_STOREOP_STORE});
    subpass.info = std::move(renderPassInfo);
    subpass.commands.push_back(std::move(drawCommand));
    auto renderpass = std::make_unique<novus::renderer::RenderpassT>();
    renderpass->sub_passes.push_back(std::move(subpass));
    scene.render_pass = std::move(renderpass);
    return scene;
}




void SampleBrowser::Begin()
{
    samples_ = {
        {
            "scene1", Scene01()
        }};

    for(auto& sample : samples_)
    {
        sample.scene.SetScene(sample.sceneInfo);
    }
    currentIndex_ = 0;
    sceneManager_.LoadScene(&samples_[currentIndex_].scene);
}
void SampleBrowser::Update(float dt)
{
    sceneManager_.Update(dt);
}
void SampleBrowser::End()
{
    sceneManager_.End();
}
void SampleBrowser::OnEvent(SDL_Event& event)
{
    sceneManager_.OnEvent(event);
}
void SampleBrowser::OnGui()
{
    ImGui::Begin("Sample Browser");

    ImGui::Text("SDL3 GPU Samples");
    ImGui::End();
}
} // namespace sample
