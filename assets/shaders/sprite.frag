#version 330 core

in vec2 texCoord;
in vec4 color;

out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
    vec4 texColor = texture(texture0, texCoord);
    //texColor.gb = vec2(texCoord.x, 0.5*sin(texCoord.y*80.0)+0.5);
    FragColor = color*texColor;
}


