#version 330 core
out vec4 FragColor;
  
uniform sampler2D tex;

in vec2 texCoord;

void main()
{

    // FragColor = vec4(0, 0.5, 1.0, 1.0) * texture(tex, texCoord);
    FragColor = vec4(0, 0.3, 1.0, 0.5);
}