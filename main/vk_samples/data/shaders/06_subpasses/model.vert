#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec2 TexCoords;

layout(binding = 0) uniform uniforms
{ 
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
} ubo;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPos, 1.0);
}