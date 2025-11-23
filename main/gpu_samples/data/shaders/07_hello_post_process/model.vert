#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;

layout(location = 0) out vec2 TexCoords;
layout(set = 0, binding = 0) readonly buffer UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(aPos, 1.0);
}