#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (location = 0) out vec2 texCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0, 1.0);
    texCoords = aTexCoords;
}