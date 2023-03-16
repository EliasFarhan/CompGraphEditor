#version 300 es
precision mediump float;

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{    
	vec3 viewDir = normalize(Position - cameraPos);
    vec3 reflectDir = reflect(viewDir, normalize(Normal));

	vec3 reflColor = texture(skybox, reflectDir).rgb;
    FragColor = vec4(reflColor,1.0);
}