

#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 transform;
uniform mat4 model;

out vec3 normal;

void main()
{
    float unitFactor = 256.0;
    vec4 position = vec4(a_position*unitFactor, 1.0);
    normal = a_normal*unitFactor-1.0;
    gl_Position = transform * model * position;
}

