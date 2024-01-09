#version 330 core

// layout (location = 0) in vec2 aPos; 
// layout (location = 1) in vec2 aTexCoords;
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec3 FragPos;
out vec2 TexCoords;

uniform mat3 model;
// uniform mat4 view;
uniform mat3 projection;
uniform float p;

void main()
{
    vec2 aPos = vertex.xy;
    vec2 aTexCoords = vec2(vertex.z + p, vertex.w);

    FragPos = model * vec3(aPos, 1.0);
    TexCoords = aTexCoords;
    gl_Position = vec4(projection * FragPos, 1.0);

    // gl_Position = projection * view * vec4(FragPos, 1.0);
}
