#include "raytracing_scene.h"

namespace gpr5300
{


core::pb::Scene RaytracingScene01()
{
    core::pb::Scene scene{};

    auto* raygenShader = scene.add_shaders();
    raygenShader->set_type(core::pb::RAY_GEN);
    raygenShader->set_path("data/shaders/rt_01/raygen.rgen.spv");


    auto* camStruct = raygenShader->add_structs();
    camStruct->set_name("CameraProperties");
    camStruct->set_size(2*sizeof(glm::mat4));
    camStruct->set_alignment(4 * alignof(float));
    auto* viewInverse = camStruct->add_attributes();
    viewInverse->set_type(core::pb::Attribute_Type_MAT4);
    viewInverse->set_stage(core::pb::RAY_GEN);
    viewInverse->set_type_name("mat4");
    viewInverse->set_name("viewInverse");
    auto* projInverse = camStruct->add_attributes();
    projInverse->set_name("projInverse");
    projInverse->set_type(core::pb::Attribute_Type_MAT4);
    projInverse->set_type_name("mat4");
    projInverse->set_stage(core::pb::RAY_GEN);

    auto* camUniform = raygenShader->add_uniforms();
    camUniform->set_name("cam");
    camUniform->set_type(core::pb::Attribute_Type_CUSTOM);
    camUniform->set_type_name("CameraProperties");
    camUniform->set_stage(core::pb::RAY_GEN);
    camUniform->set_binding(2);

    auto* accelerationStruct = raygenShader->add_uniforms();
    accelerationStruct->set_name("topLevelAS");
    accelerationStruct->set_binding(0);
    accelerationStruct->set_type(core::pb::Attribute_Type_ACCELERATION_STRUCT);
    accelerationStruct->set_type_name("accelerationStructureEXT");
    accelerationStruct->set_stage(core::pb::RAY_GEN);

    auto* imageOutput = raygenShader->add_uniforms();
    imageOutput->set_name("image");
    imageOutput->set_type_name("image2D");
    imageOutput->set_type(core::pb::Attribute_Type_IMAGE2D);
    imageOutput->set_stage(core::pb::RAY_GEN);
    imageOutput->set_binding(1);

    auto* missShader = scene.add_shaders();
    missShader->set_type(core::pb::RAY_MISS);
    missShader->set_path("data/shaders/rt_01/miss.rmiss.spv");

    auto* closestHitShader = scene.add_shaders();
    closestHitShader->set_type(core::pb::RAY_CLOSEST_HIT);
    closestHitShader->set_path("data/shaders/rt_01/closesthit.rchit.spv");

    auto* raytracingPipeline = scene.add_raytracing_pipelines();
    raytracingPipeline->set_ray_gen_shader_index(0);
    raytracingPipeline->set_miss_hit_shader_index(1);
    raytracingPipeline->set_closest_hit_shader_index(2);
    raytracingPipeline->set_max_recursion_depth(1);

    auto* accelerationStructUniform = raytracingPipeline->add_uniforms();
    accelerationStructUniform->set_type(core::pb::Attribute_Type_ACCELERATION_STRUCT);
    accelerationStructUniform->set_stage(core::pb::RAY_GEN);
    accelerationStructUniform->set_binding(0);
    accelerationStructUniform->set_name("topLevelAS");
    accelerationStructUniform->set_type_name("accelerationStructureEXT");

    auto* storageImageUniform = raytracingPipeline->add_uniforms();
    storageImageUniform->set_type(core::pb::Attribute_Type_IMAGE2D);
    storageImageUniform->set_binding(1);
    storageImageUniform->set_stage(core::pb::RAY_GEN);
    storageImageUniform->set_name("image");
    storageImageUniform->set_type_name("image2D");


    auto* quadMesh = scene.add_meshes();
    quadMesh->set_mesh_name("Quad");
    quadMesh->set_primitve_type(core::pb::Mesh_PrimitveType_QUAD);

    auto* tlas = scene.add_top_level_acceleration_structures();
    auto* blas = tlas->add_blas();
    blas->set_mesh_index(0);
    
    auto* renderPass = scene.mutable_render_pass();

    auto* subpass = renderPass->add_sub_passes();
    auto* raytracingCommand = subpass->add_raytracing_commands();
    raytracingCommand->set_pipeline_index(0);
    raytracingCommand->set_top_level_acceleration_structure_index(0);


    return scene;
}

} // namespace grp5300
