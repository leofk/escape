#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat3 model;
uniform mat3 projection;

uniform vec2 BASE;
uniform vec2 POS;

uniform float column_gap;
uniform float row_gap;

uniform int moveRight;

void main()
{
    vec2 uv = vec2(vertex.zw);

    if (moveRight == 1) {
        uv.x = 1 - uv.x;
    }

    float col_pos = ((1 - 2 * BASE.x * column_gap)/BASE.x)*uv.x + POS.x/BASE.x + column_gap;
    float row_pos = ((1 - 2 * BASE.y * row_gap)/BASE.y)*uv.y + POS.y/BASE.y + row_gap;

//     TexCoords = vec2(uv.x / BASE.x + POS.x * (1.0 / BASE.x), (uv.y / BASE.y) + POS.y * (1.0 / BASE.y));
    TexCoords = vec2(col_pos, row_pos);
    vec3 pos = projection * model * vec3(vertex.xy, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}
