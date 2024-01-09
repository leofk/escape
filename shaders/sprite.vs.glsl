#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat3 model;
uniform mat3 projection;
uniform float parallax;

void main()
{
    TexCoords = vec2(vertex.z + parallax, vertex.w);
    vec3 pos = projection * model * vec3(vertex.xy, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}