#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 FragColor;

layout(std140, set=3, binding=0) uniform slot0 {
    float ratio;
};

void main()
{
    FragColor = vec4(ratio*color, 1.0);
}