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

core::pb::Scene Scene4()
{
    core::pb::Scene scene;
    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/04_texture/texture.vert.spv");

    

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/04_texture/texture.frag.spv");
    auto* uniform = fragmentShader->add_uniforms();
    uniform->set_name("tex");
    uniform->set_type(core::pb::Attribute_Type_SAMPLER2D);
    uniform->set_type_name("sampler2D");
    uniform->set_binding(0);
    uniform->set_stage(core::pb::FRAGMENT);

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
    auto* textureMaterial = material->add_textures();
    textureMaterial->set_sampler_name("tex");
    textureMaterial->set_texture_index(0);

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

    auto* texture = scene.add_textures();
    texture->set_path("data/textures/container.jpg");
    texture->set_filter_mode(core::pb::Texture_FilteringMode_LINEAR);

    return scene;
}

core::pb::Scene Scene5()
{
    core::pb::Scene scene;
    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/05_ubo/rotated_cube.vert.spv");
    auto* constantValueStruct = vertexShader->add_structs();
    constantValueStruct->set_name("constants");
    constantValueStruct->set_alignment(16);
    constantValueStruct->set_size(64);
    auto* constantValueFloat = constantValueStruct->add_attributes();
    constantValueFloat->set_name("model");
    constantValueFloat->set_type(core::pb::Attribute_Type_MAT4);
    constantValueFloat->set_type_name("mat4");
    constantValueFloat->set_push_constant(true);
    constantValueFloat->set_stage(core::pb::VERTEX);
    auto* uboStruct = vertexShader->add_structs();
    uboStruct->set_name("uniforms");
    uboStruct->set_alignment(16);
    uboStruct->set_size(128);
    auto* viewMatrix = uboStruct->add_attributes();
    viewMatrix->set_name("view");
    viewMatrix->set_type(core::pb::Attribute_Type_MAT4);
    viewMatrix->set_type_name("mat4");
    viewMatrix->set_stage(core::pb::VERTEX);
    auto* projectionMatrix = uboStruct->add_attributes();
    projectionMatrix->set_name("projection");
    projectionMatrix->set_type(core::pb::Attribute_Type_MAT4);
    projectionMatrix->set_type_name("mat4");
    projectionMatrix->set_stage(core::pb::VERTEX);

    auto* constantUniform = vertexShader->add_uniforms();
    constantUniform->set_name("constant_values");
    constantUniform->set_type(core::pb::Attribute_Type_CUSTOM);
    constantUniform->set_type_name("constants");
    constantUniform->set_push_constant(true);
    constantUniform->set_stage(core::pb::VERTEX);

    auto* ubo = vertexShader->add_uniforms();
    ubo->set_name("ubo");
    ubo->set_type(core::pb::Attribute_Type_CUSTOM);
    ubo->set_stage(core::pb::VERTEX);
    ubo->set_type_name("uniforms");

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/05_ubo/rotated_cube.frag.spv");
    auto* textureSampler = fragmentShader->add_uniforms();
    textureSampler->set_name("tex");
    textureSampler->set_type(core::pb::Attribute_Type_SAMPLER2D);
    textureSampler->set_type_name("sampler2D");
    textureSampler->set_binding(1);
    textureSampler->set_stage(core::pb::FRAGMENT);

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_cull_face(core::pb::Pipeline_CullFace_BACK);
    pipeline->set_enable_culling(false);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_CUBE);
    mesh->set_mesh_name("Cube");

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);
    auto* textureMaterial = material->add_textures();
    textureMaterial->set_sampler_name("tex");
    textureMaterial->set_texture_index(0);

    auto* renderPass = scene.mutable_render_pass();
    auto* subPass = renderPass->add_sub_passes();
    auto* clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);

    auto* drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(36);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(false);

    auto* texture = scene.add_textures();
    texture->set_path("data/textures/container.jpg");
    texture->set_filter_mode(core::pb::Texture_FilteringMode_LINEAR);

    auto* pySystem = scene.add_py_systems();
    pySystem->set_class_("UniformSystem");
    pySystem->set_module("data.scripts.05_ubo");
    pySystem->set_path("data/scripts/05_ubo.py");

    return scene;
}

void HelloVulkanProgram::Begin() 
{
    scene_.SetScene(Scene5());

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
