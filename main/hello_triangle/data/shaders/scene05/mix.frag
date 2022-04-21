#version 310 es
precision highp float;

in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, texCoords), texture(texture2, texCoords), 0.2);
}