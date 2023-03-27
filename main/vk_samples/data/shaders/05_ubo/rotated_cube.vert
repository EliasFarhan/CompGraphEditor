#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 TexCoord;

layout(push_constant) uniform constants {
    uniform mat4 model;
} constant_values;

layout(binding = 0) uniform uniforms
{
    uniform mat4 view;
    uniform mat4 projection;
} ubo;

void main()
{
    gl_Position = ubo.projection * ubo.view * constant_values.model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}