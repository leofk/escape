#version 330

// in vec3 in_position;
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 texcoord;
uniform mat3 model;
uniform mat3 projection;

void main()
{
	vec3 pos = projection * model * vec3(vertex.xy, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    texcoord = vertex.zw;
}
