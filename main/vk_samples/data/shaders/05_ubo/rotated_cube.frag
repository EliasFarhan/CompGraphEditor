#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 TexCoord;

layout(binding = 1) uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, TexCoord);
}