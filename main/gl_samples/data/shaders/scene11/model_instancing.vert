#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout(std430, binding = 1) readonly buffer Translation
{
    vec3 pos[];
};

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float asteroidScale;


mat4 translate(vec3 pos) {
	return mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(pos.x, pos.y, pos.z, 1.0)
	);
}


void main()
{ 
	mat4 instanceMatrix = mat4(1.0);
	instanceMatrix = instanceMatrix * translate(pos[gl_InstanceID]);
    TexCoords = aTexCoords;    
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}