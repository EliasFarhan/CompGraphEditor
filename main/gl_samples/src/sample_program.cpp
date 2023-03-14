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

    constexpr std::string_view modelPath = "data/model/rock/rock.obj";
    *scene.add_model_paths() = modelPath;

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_MODEL);
    mesh->set_model_index(0);
    mesh->set_model_path(modelPath.data());
    mesh->set_mesh_name("Cube");

    auto* texture = scene.add_textures();
    texture->set_path("data/model/rock/rock.png");
    texture->set_filter_mode(core::pb::Texture_FilteringMode_LINEAR);


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

core::pb::Scene Scene8()
{
    core::pb::Scene scene;
    auto* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::Shader_Type_VERTEX);
    vertexShader->set_path("data/shaders/scene08/model.vert");
    auto* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    fragmentShader->set_path("data/shaders/scene08/model.frag");
    auto* postVertexShader = scene.add_shaders();
    postVertexShader->set_type(core::pb::Shader_Type_VERTEX);
    postVertexShader->set_path("data/shaders/scene08/screen.vert");
    auto* postFragmentShader = scene.add_shaders();
    postFragmentShader->set_type(core::pb::Shader_Type_FRAGMENT);
    postFragmentShader->set_path("data/shaders/scene08/screen_edge_detection.frag");

    auto* pipeline = scene.add_pipelines();
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_mask(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL);

    auto* postProcessPipeline = scene.add_pipelines();
    postProcessPipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    postProcessPipeline->set_vertex_shader_index(2);
    postProcessPipeline->set_fragment_shader_index(3);
    postProcessPipeline->set_depth_test_enable(true);
    postProcessPipeline->set_depth_mask(true);
    postProcessPipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS_OR_EQUAL);
    
    auto* defaultMaterial = scene.add_materials();
    defaultMaterial->set_pipeline_index(0);
    auto* diffuseTexture = defaultMaterial->add_textures();
    diffuseTexture->set_sampler_name("texture_diffuse1");
    diffuseTexture->set_texture_index(0);

    auto* postProcessMaterial = scene.add_materials();
    postProcessMaterial->set_pipeline_index(1);
    auto* postProcessTexture = postProcessMaterial->add_textures();
    postProcessTexture->set_framebuffer_name("FirstPassFramebuffer");
    postProcessTexture->set_sampler_name("screenTexture");
    postProcessTexture->set_attachment_name("ColorBuffer");
    postProcessTexture->set_texture_index(-1);

    auto* framebuffer = scene.add_framebuffers();
    framebuffer->set_name("FirstPassFramebuffer");
    auto* renderTarget = framebuffer->add_color_attachments();
    renderTarget->set_format(core::pb::RenderTarget_Format_RGBA);
    renderTarget->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_8);
    renderTarget->set_size_type(core::pb::RenderTarget_Size_WINDOW_SIZE);
    renderTarget->set_type(core::pb::RenderTarget_Type_UNSIGNED);
    renderTarget->set_name("ColorBuffer");

    auto* depth = framebuffer->mutable_depth_stencil_attachment();
    depth->set_format(core::pb::RenderTarget_Format_DEPTH_STENCIL);
    depth->set_size_type(core::pb::RenderTarget_Size_WINDOW_SIZE);
    depth->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_24);
    depth->set_rbo(true);
    depth->set_type(core::pb::RenderTarget_Type_UNSIGNED);
    depth->set_name("DepthRBO");

    auto* modelTexture = scene.add_textures();
    modelTexture->set_type(core::pb::DIFFUSE);
    modelTexture->set_filter_mode(core::pb::Texture_FilteringMode_LINEAR);
    modelTexture->set_path("data/textures/container.jpg");

    auto* renderPass = scene.mutable_render_pass();
    auto* firstPass = renderPass->add_sub_passes();
    firstPass->set_framebuffer_index(0);
    auto* cubeCommand = firstPass->add_commands();
    cubeCommand->set_count(36);
    cubeCommand->set_automatic_draw(false);
    cubeCommand->set_draw_elements(true);
    cubeCommand->set_material_index(0);
    cubeCommand->set_mesh_index(0);

    auto* postProcessPass = renderPass->add_sub_passes();
    postProcessPass->set_framebuffer_index(-1);
    auto* postProcessCommand = postProcessPass->add_commands();
    postProcessCommand->set_automatic_draw(true);
    postProcessCommand->set_count(6);
    postProcessCommand->set_draw_elements(true);
    postProcessCommand->set_material_index(1);
    postProcessCommand->set_mesh_index(1);

    auto* cubeMesh = scene.add_meshes();
    cubeMesh->set_primitve_type(core::pb::Mesh_PrimitveType_CUBE);

    auto* quadMesh = scene.add_meshes();
    quadMesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto* cameraPySystem = scene.add_py_systems();
    cameraPySystem->set_path("data/scripts/camera.py");
    cameraPySystem->set_class_("Camera");
    cameraPySystem->set_module("data.scripts.camera");

    auto* scenePySystem = scene.add_py_systems();
    scenePySystem->set_path("data/scripts/scene08.py");
    scenePySystem->set_class_("Scene08");
    scenePySystem->set_module("data.scripts.scene08");

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
        {"scene8", Scene8()},
    };
    for(auto& sample : samples_)
    {
        sample.scene.SetScene(sample.sceneInfo);
    }

    sceneManager_.LoadScene(&samples_.back().scene);
    currentIndex_ = samples_.size() - 1;
}

void SampleBrowserProgram::Update(float dt)
{
    sceneManager_.Update(dt);
}

void SampleBrowserProgram::End()
{
    sceneManager_.End();
}
void SampleBrowserProgram::OnGui()
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

void SampleBrowserProgram::OnEvent(SDL_Event& event)
{
    sceneManager_.OnEvent(event);
}
} // namespace gpr5300
