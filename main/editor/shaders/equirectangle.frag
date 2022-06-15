#version 300 es
precision highp float;

out vec4 FragColor;

in vec2 TexCoords;

uniform samplerCube irradiance;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec3 equirectangle_to_world(vec2 uv)
{
    float y = sin((uv.y-0.5)/invAtan.y);
    float x = cos((uv.y-0.5)/invAtan.x)*cos((uv.x-0.5)/invAtan.x);
    float z = cos((uv.y-0.5)/invAtan.x)*sin((uv.x-0.5)/invAtan.x);
    return vec3(x, y, z);
}

void main()
{
    vec3 color = texture(irradiance, equirectangle_to_world(TexCoords)).xyz;
    FragColor = vec4(color, 1.0);
}