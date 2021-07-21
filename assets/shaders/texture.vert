#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texture;
layout (location = 3) in vec3 a_normal;

uniform mat4 transform;

out vec2 textureCoords;
out vec3 normal;
out vec4 color;

void main()
{
    textureCoords = a_texture;
    normal = a_normal;
    color = a_color;
    gl_Position = transform * vec4(a_position, 1.0);
}


