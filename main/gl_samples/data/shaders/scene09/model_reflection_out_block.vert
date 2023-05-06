#version 300 es
precision mediump float;


layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

struct out_block
{
    vec2 TexCoords;
    vec3 Normal;
    vec3 Position;
};

out out_block outputs;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMatrix;

void main()
{
    outputs.TexCoords = aTexCoords;
	outputs.Normal = mat3(normalMatrix) * aNormal;
    outputs.Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}