//
// Created by efarhan on 12/28/22.
//

#include "triangle_program.h"

namespace gpr5300
{

pb::Scene Scene1()
{
    pb::Scene scene;

    pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/01_triangle/triangle.vert.spv");

    pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/01_triangle/triangle.frag.spv");

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(pb::Pipeline_Type_RASTERIZE);

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);

    auto* renderPass = scene.mutable_render_pass();
    auto* subPass = renderPass->add_sub_passes();
    auto* clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);

    auto* drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(3);
    drawCommand->set_mesh_index(-1);
    drawCommand->set_draw_elements(false);
    drawCommand->set_mode(pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(true);
    return scene;
}

void HelloVulkanProgram::Begin() 
{
    scene_.SetScene(Scene1());

    sceneManager_.LoadScene(&scene_);
}
void HelloVulkanProgram::Update(float dt)
{
    sceneManager_.Update(dt);
}
void HelloVulkanProgram::End()
{
    sceneManager_.End();
}
void HelloVulkanProgram::DrawImGui()
{
    
}

}