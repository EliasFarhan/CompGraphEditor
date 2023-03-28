#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 aPos;

layout(location = 0) out vec3 color;

layout(push_constant) uniform constants {
    float value;
} constant_values;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0, 1.0);
    color = vec3(abs(aPos.x+0.5), abs(aPos.y+0.5), 1.0)*constant_values.value;
}