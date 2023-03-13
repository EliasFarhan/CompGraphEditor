#include "sample_program.h"
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
    pySystem->set_path("data/scripts/scene03.py");

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

/**
 * @brief Scene 06 is a sample with rotating cubes, showcasing correct depth testing
*/
core::pb::Scene Scene6()
{
    core::pb::Scene scene;

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene06/rotated_cube.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene06/rotated_cube.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL);
    pipeline->set_depth_mask(true);
    

    auto* texture = scene.add_textures();
    texture->set_path("data/textures/container.jpg");

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);
    auto* materialTexture = material->add_textures();
    materialTexture->set_sampler_name("tex");
    materialTexture->set_texture_index(0);

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_CUBE);

    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(36);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(false);

    auto* pySystem = scene.add_py_systems();
    pySystem->set_class_("Scene06");
    pySystem->set_module("data.scripts.scene06");
    pySystem->set_path("data/scripts/scene06.py");
    return scene;
}

core::pb::Scene Scene7()
{
    core::pb::Scene scene;

    core::pb::Shader *vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene07/rock.vert");

    core::pb::Shader *fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene07/rock.frag");

    auto *pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL);
    pipeline->set_depth_mask(true);

    auto* material = scene.add_materials();
    material->set_pipeline_index(0);

    auto* materialTexture = material->add_textures();
    materialTexture->set_texture_index(0);
    materialTexture->set_sampler_name("tex");

    constexpr std::string_view modelPath = "data/model/rock.obj";
    *scene.add_model_paths() = modelPath;

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_MODEL);
    mesh->set_model_index(0);
    mesh->set_model_path(modelPath.data());
    mesh->set_mesh_name("Cube");

    auto* texture = scene.add_textures();
    texture->set_path("data/model/rock.png");


    auto *renderPass = scene.mutable_render_pass();
    auto *subPass = renderPass->add_sub_passes();
    auto *clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto *drawCommand = subPass->add_commands();
    drawCommand->set_material_index(0);
    drawCommand->set_count(192*3);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(false);

    auto* cameraPySystem = scene.add_py_systems();
    cameraPySystem->set_path("data/scripts/camera.py");
    cameraPySystem->set_class_("Camera");
    cameraPySystem->set_module("data.scripts.camera");

    auto* scenePySystem = scene.add_py_systems();
    scenePySystem->set_path("data/scripts/scene07.py");
    scenePySystem->set_class_("Scene07");
    scenePySystem->set_module("data.scripts.scene07");

    return scene;
}

void SampleBrowserProgram::Begin()
{
    samples_ = {
        {"scene1", Scene1()},
        {"scene2", Scene2()},
        {"scene3", Scene3()},
        {"scene4", Scene4()},
        {"scene5", Scene5()},
        {"scene6", Scene6()},
        {"scene7", Scene7()},
    };
    for(auto& sample : samples_)
    {
        sample.scene.SetScene(sample.sceneInfo);
    }

    sceneManager_.LoadScene(&samples_[0].scene);
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
    if(ImGui::BeginCombo("Samples", samples_[currentIndex_].sceneName.data()))
    {
        for(std::size_t i = 0; i < samples_.size(); i++)
        {
            if(ImGui::Selectable(samples_[i].sceneName.data(), i == currentIndex_))
            {
                sceneManager_.LoadScene(&samples_[i].scene);
                currentIndex_ = i;
            }
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}
} // namespace gpr5300
