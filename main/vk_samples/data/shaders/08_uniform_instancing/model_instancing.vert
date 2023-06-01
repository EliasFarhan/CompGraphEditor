#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

layout (location = 0) out vec2 TexCoords;


layout(binding = 0) uniform uniforms
{
    uniform mat4 view;
    uniform mat4 projection;
	uniform vec3 pos[1000];
} ubo;


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
	instanceMatrix = instanceMatrix * translate(ubo.pos[gl_InstanceIndex]);
    TexCoords = aTexCoords;    
    gl_Position = ubo.projection * ubo.view * instanceMatrix * vec4(aPos, 1.0);
}