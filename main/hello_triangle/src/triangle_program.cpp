#include "triangle_program.h"

namespace gpr5300
{

pb::Scene Scene1()
{
    pb::Scene scene;

    pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene01/triangle.vert");

    pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene01/triangle.frag");

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
    drawCommand->set_count(3);
    drawCommand->set_mesh_index(-1);
    drawCommand->set_draw_elements(false);
    drawCommand->set_mode(pb::DrawCommand_Mode_TRIANGLES);
    return scene;
}

pb::Scene Scene2()
{
    pb::Scene scene;

    pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene02/quad.vert");

    pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene02/quad.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(pb::Pipeline_Type_RASTERIZE);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);



    auto *drawCommand = subPass->add_commands();
    drawCommand->set_pipeline_index(0);
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(pb::DrawCommand_Mode_TRIANGLES);
    return scene;
}


pb::Scene Scene3()
{
    pb::Scene scene;

    pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene03/uniform.vert");

    pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene03/uniform.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(pb::Pipeline_Type_RASTERIZE);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(pb::Mesh_PrimitveType_QUAD);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_pipeline_index(0);
    drawCommand->set_count(6);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(pb::DrawCommand_Mode_TRIANGLES);

    auto* pySystem = scene.add_py_systems();
    pySystem->set_class_("Scene03");
    pySystem->set_module("data.scripts.scene03");

    return scene;
}


void HelloTriangleProgram::Begin()
{
    scene_.SetScene(Scene3());

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
