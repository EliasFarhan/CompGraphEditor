//
// Created by efarhan on 12/28/22.
//

#include "sample_program.h"

#include <imgui.h>

namespace gpr5300
{

core::pb::Scene Scene1()
{
    core::pb::Scene scene;

    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/01_triangle/triangle.vert.spv");

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
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


core::pb::Scene Scene2()
{
    core::pb::Scene scene;
    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/02_quad/quad.vert.spv");

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/02_quad/quad.frag.spv");

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_cull_face(core::pb::Pipeline_CullFace_BACK);
    pipeline->set_enable_culling(false);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);
    mesh->set_mesh_name("Quad");

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
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(true);

    return scene;
}

core::pb::Scene Scene3()
{
    core::pb::Scene scene;
    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/03_push_constant/uniform.vert.spv");
    auto* constantValueStruct = vertexShader->add_structs();
    constantValueStruct->set_name("constants");
    constantValueStruct->set_alignment(4);
    constantValueStruct->set_size(4);
    auto* constantValueFloat = constantValueStruct->add_attributes();
    constantValueFloat->set_name("value");
    constantValueFloat->set_type(core::pb::Attribute_Type_FLOAT);
    constantValueFloat->set_type_name("float");
    constantValueFloat->set_push_constant(true);
    constantValueFloat->set_stage(core::pb::VERTEX);

    auto* uniform = vertexShader->add_uniforms();
    uniform->set_name("constant_values");
    uniform->set_type(core::pb::Attribute_Type_CUSTOM);
    uniform->set_type_name("constants");
    uniform->set_push_constant(true);
    uniform->set_stage(core::pb::VERTEX);

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/03_push_constant/uniform.frag.spv");

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_cull_face(core::pb::Pipeline_CullFace_BACK);
    pipeline->set_enable_culling(false);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);
    mesh->set_mesh_name("Quad");

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
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(false);


    auto* pySystem = scene.add_py_systems();
    pySystem->set_class_("UniformSystem");
    pySystem->set_module("data.scripts.03_uniform");
    pySystem->set_path("data/scripts/03_uniform.py");

    return scene;
}

void HelloVulkanProgram::Begin() 
{
    scene_.SetScene(Scene3());

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
