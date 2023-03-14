//
// Created by efarhan on 12/28/22.
//

#include "triangle_program.h"

#include <imgui.h>

namespace gpr5300
{

core::pb::Scene Scene1()
{
    core::pb::Scene scene;

    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/01_triangle/triangle.vert.spv");

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/01_triangle/triangle.frag.spv");

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_cull_face(core::pb::Pipeline_CullFace_FRONT);

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
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
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
void HelloVulkanProgram::OnGui()
{
    ImGui::Begin("Hello Vulkan");
    ImGui::End();
}

}
