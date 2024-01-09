#version 330 core
in vec2 TexCoords;
layout(location = 0) out vec4 color;

uniform sampler2D tex;
uniform vec3 spriteColor;
uniform int isLock;

void main()
{
    vec3 shade_offset = vec3(0, 0, 0);
    color = vec4(spriteColor + shade_offset, 1.0) * texture(tex, TexCoords);
}