

#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in ivec2 a_texcoord;

uniform mat4 transform;
uniform mat4 model;

out vec3 normal;
out vec2 texCoord;

void main()
{
    float unitFactor = 256.0;
    float halfPix = 1.0/1024.0;  
    vec4 position = vec4(a_position*unitFactor, 1.0);

    normal = a_normal*unitFactor-1.0;
    texCoord = vec2(a_texcoord.x/255.0+halfPix, a_texcoord.y/255.0+halfPix);

    gl_Position = transform * model * position;
}

