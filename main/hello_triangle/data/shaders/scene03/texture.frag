#version 310 es
precision highp float;

in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

uniform textureSampler tex;

void main()
{
    FragColor = texture(tex, texCoords);
}