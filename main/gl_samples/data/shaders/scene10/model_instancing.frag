#version 300 es
precision mediump float;

out vec4 FragColor;

in vec2 TexCoords;

struct Material 
{
	sampler2D texture_diffuse1;
};

uniform Material material;

void main()
{    
    FragColor = texture(material.texture_diffuse1, TexCoords);
}