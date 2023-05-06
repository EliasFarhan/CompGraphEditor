#version 300 es

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

struct uniforms
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

uniform uniforms ubo;

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}