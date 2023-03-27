#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, texCoords);
}