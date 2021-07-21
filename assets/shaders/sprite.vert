#version 330 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texCoord;

out vec2 texCoord;
out vec4 color;

uniform mat3 transform;

void main()
{
    texCoord = a_texCoord;

    color = a_color;
    gl_Position = vec4(transform * vec3(a_position, 1.0), 1.0);
}

