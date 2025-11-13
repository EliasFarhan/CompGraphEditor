#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 TexCoord;

layout(set = 0, binding = 1) readonly buffer Transform
{
    mat4[] model;
}
transform;

layout(set = 0, binding = 0) readonly buffer CameraBuffer
{
    mat4 view;
    mat4 projection;
} camera;

void main()
{
    gl_Position = camera.projection * camera.view * transform.model[gl_InstanceIndex] * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}