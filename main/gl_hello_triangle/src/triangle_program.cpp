#include "triangle_program.h"
#include <imgui.h>

namespace gpr5300
{

core::pb::Scene Scene1()
{
    core::pb::Scene scene;

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene01/triangle.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene01/triangle.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);

    auto *drawCommand = subPass->add_commands();
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

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene02/quad.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene02/quad.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);



    auto *drawCommand = subPass->add_commands();
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

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene03/uniform.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene03/uniform.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(true);

    auto* pySystem = scene.add_py_systems();
    pySystem->set_class_("Scene03");
    pySystem->set_module("data.scripts.scene03");

    return scene;
}


core::pb::Scene Scene4()
{
    core::pb::Scene scene;

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene04/texture.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene04/texture.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);

    auto* texture = scene.add_textures();
    texture->set_path("data/textures/container.jpg");

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);
    auto* materialTexture = material->add_textures();
    materialTexture->set_sampler_name("tex");
    materialTexture->set_texture_index(0);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(true);
    return scene;
}


core::pb::Scene Scene5()
{
    core::pb::Scene scene;

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene05/mix.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene05/mix.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);

    auto* texture1 = scene.add_textures();
    texture1->set_path("data/textures/container.jpg");
    auto* texture2 = scene.add_textures();
    texture2->set_path("data/textures/awesomeface.png");

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);
    auto* materialTexture1 = material->add_textures();
    materialTexture1->set_sampler_name("texture1");
    materialTexture1->set_texture_index(0);
    auto* materialTexture2 = material->add_textures();
    materialTexture2->set_sampler_name("texture2");
    materialTexture2->set_texture_index(1);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(true);
    return scene;
}


void SampleBrowserProgram::Begin()
{
    scene_.SetScene(Scene5());

    sceneManager_.LoadScene(&scene_);
}

void SampleBrowserProgram::Update(float dt)
{
    sceneManager_.Update(dt);
}

void SampleBrowserProgram::End()
{
    sceneManager_.End();
}
void SampleBrowserProgram::DrawImGui()
{
    ImGui::Begin("Sample Browser");
    ImGui::End();
}
} // namespace gpr5300
