#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 TexCoords;

layout(binding = 1) uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);
}