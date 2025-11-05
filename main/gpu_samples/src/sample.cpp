#include "sample.h"

#include <generated/renderer_generated.h>

namespace sample
{


void SampleBrowser::Begin() {}
void SampleBrowser::Update(float dt) {}
void SampleBrowser::End() {}
void SampleBrowser::OnEvent(SDL_Event& event) {}
void SampleBrowser::OnGui() {}

novus::renderer::SceneT Scene01()
{
    novus::renderer::SceneT scene{};
    scene.shaders.push_back({.stage = novus::internal::ShaderStage_VERTEX, .path = "data/shaders/triangle.vert"});
    scene.shaders.push_back({.stage = novus::internal::ShaderStage_FRAGMENT, .path = "data/shaders/triangle.frag"});
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


    graphicsPipeline.info = std::move(graphicsPipelineInfo);

    return scene;
}
} // namespace sample
