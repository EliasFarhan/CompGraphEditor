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

    auto* mesh = scene.add_meshes();
    mesh->set_primitve_type(core::pb::Mesh_PrimitveType_NONE);

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
    drawCommand->set_mesh_index(0);
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
    auto* posInput = pipeline->add_in_vertex_attributes();
    posInput->set_name("pos");
    posInput->set_binding(0);
    posInput->set_stage(core::pb::VERTEX);
    posInput->set_type(core::pb::Attribute_Type_VEC3);
    posInput->set_type_name("vec3");

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

    auto* posInput = pipeline->add_in_vertex_attributes();
    posInput->set_name("pos");
    posInput->set_binding(0);
    posInput->set_stage(core::pb::VERTEX);
    posInput->set_type(core::pb::Attribute_Type_VEC3);
    posInput->set_type_name("vec3");

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


    auto* pySystem = scene.add_systems();
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
    auto* posInput = pipeline->add_in_vertex_attributes();
    posInput->set_name("pos");
    posInput->set_binding(0);
    posInput->set_stage(core::pb::VERTEX);
    posInput->set_type(core::pb::Attribute_Type_VEC3);
    posInput->set_type_name("vec3");
    auto* texCoords = pipeline->add_in_vertex_attributes();
    texCoords->set_name("texCoords");
    texCoords->set_binding(0);
    texCoords->set_stage(core::pb::VERTEX);
    texCoords->set_type(core::pb::Attribute_Type_VEC2);
    texCoords->set_type_name("vec2");

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
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS);
    pipeline->set_depth_mask(true);

    auto* posInput = pipeline->add_in_vertex_attributes();
    posInput->set_name("pos");
    posInput->set_binding(0);
    posInput->set_stage(core::pb::VERTEX);
    posInput->set_type(core::pb::Attribute_Type_VEC3);
    posInput->set_type_name("vec3");
    auto* texCoords = pipeline->add_in_vertex_attributes();
    texCoords->set_name("texCoords");
    texCoords->set_binding(0);
    texCoords->set_stage(core::pb::VERTEX);
    texCoords->set_type(core::pb::Attribute_Type_VEC2);
    texCoords->set_type_name("vec2");

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

    auto* pySystem = scene.add_systems();
    pySystem->set_class_("UniformSystem");
    pySystem->set_module("data.scripts.05_ubo");
    pySystem->set_path("data/scripts/05_ubo.py");

    return scene;
}

core::pb::Scene Scene6()
{
    core::pb::Scene scene;
    auto* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/06_subpasses/model.vert.spv");
    auto* uboStruct = vertexShader->add_structs();
    uboStruct->set_name("uniforms");
    uboStruct->set_alignment(16);
    uboStruct->set_size(sizeof(glm::mat4)*3);
    auto* modelAttr = uboStruct->add_attributes();
    modelAttr->set_name("model");
    modelAttr->set_type(core::pb::Attribute_Type_MAT4);
    modelAttr->set_type_name("mat4");
    modelAttr->set_stage(core::pb::VERTEX);
    auto* viewAttr = uboStruct->add_attributes();
    viewAttr->set_name("view");
    viewAttr->set_type(core::pb::Attribute_Type_MAT4);
    viewAttr->set_type_name("mat4");
    viewAttr->set_stage(core::pb::VERTEX);
    auto* projectionAttr = uboStruct->add_attributes();
    projectionAttr->set_name("projection");
    projectionAttr->set_type(core::pb::Attribute_Type_MAT4);
    projectionAttr->set_type_name("mat4");
    projectionAttr->set_stage(core::pb::VERTEX);
    auto* ubo = vertexShader->add_uniforms();
    ubo->set_type(core::pb::Attribute_Type_CUSTOM);
    ubo->set_stage(core::pb::VERTEX);
    ubo->set_type_name("uniforms");
    ubo->set_binding(0);
    ubo->set_name("ubo");


    auto* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/06_subpasses/model.frag.spv");
    auto* diffuseSampler = fragmentShader->add_uniforms();
    diffuseSampler->set_type(core::pb::Attribute_Type_SAMPLER2D);
    diffuseSampler->set_type_name("sampler2D");
    diffuseSampler->set_name("texture_diffuse1");
    diffuseSampler->set_binding(1);
    diffuseSampler->set_stage(core::pb::FRAGMENT);

    auto* postVertexShader = scene.add_shaders();
    postVertexShader->set_type(core::pb::VERTEX);
    postVertexShader->set_path("data/shaders/06_subpasses/screen.vert.spv");

    auto* postFragmentShader = scene.add_shaders();
    postFragmentShader->set_type(core::pb::FRAGMENT);
    postFragmentShader->set_path("data/shaders/06_subpasses/screen_edge_detection.frag.spv");
    auto* screenSampler = postFragmentShader->add_uniforms();
    screenSampler->set_type(core::pb::Attribute_Type_SAMPLER2D);
    screenSampler->set_stage(core::pb::FRAGMENT);
    screenSampler->set_name("screenTexture");
    screenSampler->set_type_name("sampler2D");
    screenSampler->set_binding(1);

    auto* pipeline = scene.add_pipelines();
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_mask(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS);
    {
        auto* posInput = pipeline->add_in_vertex_attributes();
        posInput->set_name("pos");
        posInput->set_binding(0);
        posInput->set_stage(core::pb::VERTEX);
        posInput->set_type(core::pb::Attribute_Type_VEC3);
        posInput->set_type_name("vec3");
        auto* texCoords = pipeline->add_in_vertex_attributes();
        texCoords->set_name("texCoords");
        texCoords->set_binding(0);
        texCoords->set_stage(core::pb::VERTEX);
        texCoords->set_type(core::pb::Attribute_Type_VEC2);
        texCoords->set_type_name("vec2");
    }
    auto* postProcessPipeline = scene.add_pipelines();
    postProcessPipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    postProcessPipeline->set_vertex_shader_index(2);
    postProcessPipeline->set_fragment_shader_index(3);
    {
        auto* posInput = postProcessPipeline->add_in_vertex_attributes();
        posInput->set_name("pos");
        posInput->set_binding(0);
        posInput->set_stage(core::pb::VERTEX);
        posInput->set_type(core::pb::Attribute_Type_VEC3);
        posInput->set_type_name("vec3");
        auto* texCoords = postProcessPipeline->add_in_vertex_attributes();
        texCoords->set_name("texCoords");
        texCoords->set_binding(0);
        texCoords->set_stage(core::pb::VERTEX);
        texCoords->set_type(core::pb::Attribute_Type_VEC2);
        texCoords->set_type_name("vec2");
    }

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
    renderTarget->set_type(core::pb::RenderTarget_Type_FLOAT);
    renderTarget->set_name("ColorBuffer");


    auto* depth = framebuffer->mutable_depth_stencil_attachment();
    depth->set_format(core::pb::RenderTarget_Format_DEPTH_STENCIL);
    depth->set_size_type(core::pb::RenderTarget_Size_WINDOW_SIZE);
    depth->set_format_size(core::pb::RenderTarget_FormatSize_SIZE_32);
    depth->set_rbo(true);
    depth->set_type(core::pb::RenderTarget_Type_FLOAT);
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
    cubeCommand->set_name("CubeCommand");

    auto* sphereCommand = firstPass->add_commands();
    sphereCommand->set_count(core::sphereIndices);
    sphereCommand->set_automatic_draw(false);
    sphereCommand->set_draw_elements(true);
    sphereCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLE_STRIP);
    sphereCommand->set_material_index(0);
    sphereCommand->set_mesh_index(2);
    sphereCommand->set_name("SphereCommand");

    auto* postProcessPass = renderPass->add_sub_passes();
    postProcessPass->set_framebuffer_index(-1);
    auto* postProcessCommand = postProcessPass->add_commands();
    postProcessCommand->set_automatic_draw(true);
    postProcessCommand->set_count(6);
    postProcessCommand->set_draw_elements(true);
    postProcessCommand->set_material_index(1);
    postProcessCommand->set_mesh_index(1);
    postProcessCommand->set_name("PostProcess");

    auto* cubeMesh = scene.add_meshes();
    cubeMesh->set_primitve_type(core::pb::Mesh_PrimitveType_CUBE);

    auto* quadMesh = scene.add_meshes();
    quadMesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto* sphereMesh = scene.add_meshes();
    sphereMesh->set_primitve_type(core::pb::Mesh_PrimitveType_SPHERE);

    auto* scale = quadMesh->mutable_scale();
    scale->set_x(2.0f);
    scale->set_y(2.0f);
    scale->set_z(2.0f);
    auto* cameraPySystem = scene.add_systems();
    cameraPySystem->set_path("data/scripts/camera.py");
    cameraPySystem->set_class_("CameraSystem");
    cameraPySystem->set_module(core::nativeModuleName.data());

    auto* scenePySystem = scene.add_systems();
    scenePySystem->set_path("data/scripts/06_subpasses.py");
    scenePySystem->set_class_("SubpassSystem");
    scenePySystem->set_module("data.scripts.06_subpasses");
    
    return scene;
}

core::pb::Scene Scene07()
{
    core::pb::Scene scene;

    core::pb::Shader* vertexShader = scene.add_shaders();
    vertexShader->set_type(core::pb::VERTEX);
    vertexShader->set_path("data/shaders/07_model/rock.vert.spv");
    auto* uboStruct = vertexShader->add_structs();
    uboStruct->set_name("uniforms");
    uboStruct->set_alignment(16);
    uboStruct->set_size(sizeof(glm::mat4) * 3);
    auto* modelAttr = uboStruct->add_attributes();
    modelAttr->set_name("model");
    modelAttr->set_type(core::pb::Attribute_Type_MAT4);
    modelAttr->set_type_name("mat4");
    modelAttr->set_stage(core::pb::VERTEX);
    auto* viewAttr = uboStruct->add_attributes();
    viewAttr->set_name("view");
    viewAttr->set_type(core::pb::Attribute_Type_MAT4);
    viewAttr->set_type_name("mat4");
    viewAttr->set_stage(core::pb::VERTEX);
    auto* projectionAttr = uboStruct->add_attributes();
    projectionAttr->set_name("projection");
    projectionAttr->set_type(core::pb::Attribute_Type_MAT4);
    projectionAttr->set_type_name("mat4");
    projectionAttr->set_stage(core::pb::VERTEX);
    auto* ubo = vertexShader->add_uniforms();
    ubo->set_type(core::pb::Attribute_Type_CUSTOM);
    ubo->set_stage(core::pb::VERTEX);
    ubo->set_type_name("uniforms");
    ubo->set_binding(0);
    ubo->set_name("ubo");

    core::pb::Shader* fragmentShader = scene.add_shaders();
    fragmentShader->set_type(core::pb::FRAGMENT);
    fragmentShader->set_path("data/shaders/07_model/rock.frag.spv");
    auto* diffuseSampler = fragmentShader->add_uniforms();
    diffuseSampler->set_type(core::pb::Attribute_Type_SAMPLER2D);
    diffuseSampler->set_type_name("sampler2D");
    diffuseSampler->set_name("tex");
    diffuseSampler->set_binding(1);
    diffuseSampler->set_stage(core::pb::FRAGMENT);

    auto* pipeline = scene.add_pipelines();
    pipeline->set_vertex_shader_index(0);
    pipeline->set_fragment_shader_index(1);
    pipeline->set_type(core::pb::Pipeline_Type_RASTERIZE);
    pipeline->set_depth_test_enable(true);
    pipeline->set_depth_compare_op(core::pb::Pipeline_DepthCompareOp_LESS);
    pipeline->set_depth_mask(true);
    auto* posInput = pipeline->add_in_vertex_attributes();
    posInput->set_name("pos");
    posInput->set_binding(0);
    posInput->set_stage(core::pb::VERTEX);
    posInput->set_type(core::pb::Attribute_Type_VEC3);
    posInput->set_type_name("vec3");
    auto* texCoords = pipeline->add_in_vertex_attributes();
    texCoords->set_name("texCoords");
    texCoords->set_binding(0);
    texCoords->set_stage(core::pb::VERTEX);
    texCoords->set_type(core::pb::Attribute_Type_VEC2);
    texCoords->set_type_name("vec2");


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
    mesh->set_mesh_name("Cube");

    auto* texture = scene.add_textures();
    texture->set_path("data/model/rock/rock.png");
    texture->set_filter_mode(core::pb::Texture_FilteringMode_LINEAR);


    auto* renderPass = scene.mutable_render_pass();
    auto* subPass = renderPass->add_sub_passes();
    auto* clearColor = subPass->mutable_clear_color();
    clearColor->set_r(0.0f);
    clearColor->set_g(0.0f);
    clearColor->set_b(0.0f);
    clearColor->set_a(0.0f);


    auto* drawCommand = subPass->add_commands();

    drawCommand->set_material_index(0);
    drawCommand->set_count(192 * 3);
    drawCommand->set_mesh_index(0);
    drawCommand->set_draw_elements(true);
    drawCommand->set_mode(core::pb::DrawCommand_Mode_TRIANGLES);
    drawCommand->set_automatic_draw(false);

    auto* cameraPySystem = scene.add_systems();
    cameraPySystem->set_path("data/scripts/camera.py");
    cameraPySystem->set_class_("CameraSystem");
    cameraPySystem->set_module(core::nativeModuleName.data());

    auto* scenePySystem = scene.add_systems();
    scenePySystem->set_path("data/scripts/07_model.py");
    scenePySystem->set_class_("Scene07");
    scenePySystem->set_module("data.scripts.07_model");

    return scene;
}

void HelloVulkanProgram::Begin() 
{
    scene_.SetScene(Scene6());

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
}

void HelloVulkanProgram::OnEvent(SDL_Event& event)
{
    sceneManager_.OnEvent(event);
}
}
