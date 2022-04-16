#include "triangle_program.h"

namespace gpr5300
{
void HelloTriangleProgram::Begin()
{
    pb::Scene scene;

    pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/triangle.vert");

    pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/triangle.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(pb::Pipeline_Type_RASTERIZE);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);
    auto *drawCommand = subPass->add_commands();
    drawCommand->set_pipeline_index(0);
    drawCommand->set_first(0);
    drawCommand->set_count(3);
    drawCommand->set_mode(pb::DrawCommand_Mode_TRIANGLES);

    scene_.SetScene(scene);

    sceneManager_.LoadScene(&scene_);
}

void HelloTriangleProgram::Update(float dt)
{
    sceneManager_.Update(dt);
}

void HelloTriangleProgram::End()
{
    sceneManager_.End();
}
} // namespace gpr5300
