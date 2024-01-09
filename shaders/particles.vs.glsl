#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 tCoord;
layout (location = 2) in vec2 aOffset;

uniform mat3 projection;
uniform mat3 player_pos;

out vec2 texCoord;

void main()
{

    vec3 player_center = player_pos * vec3(aPos, 1.0);
    vec3 model_space_pos = (player_center + vec3(aOffset, 0.0));
    gl_Position = vec4(projection * model_space_pos, 1.0);
    texCoord = tCoord;
}  