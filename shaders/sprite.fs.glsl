#version 330 core
in vec2 TexCoords;
layout(location = 0) out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;
uniform int moveRight;

void main()
{
    if (moveRight == 0) {
        color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
    } else {
        color = vec4(spriteColor, 1.0) * texture(image, vec2(-(TexCoords.x-1), TexCoords.y));
//        color = vec4(1.0, 0.0, 0.0, 1.0) * texture(image, vec2(0.5, 0.5));
    }
}