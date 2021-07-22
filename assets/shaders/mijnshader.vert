

#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in ivec2 a_texcoord;
layout (location = 3) in vec4 a_color;

uniform mat4 transform;
uniform mat4 model;

out vec3 normal;
out vec2 texCoord;
out vec4 color;

void main()
{
    float unitFactor = 256.0;
    //float halfPix = 1.0/2048.0;  
    float halfPix = 0.0;
    vec4 position = vec4(a_position*unitFactor, 1.0);

    normal = a_normal*unitFactor-1.0;
    texCoord = vec2(a_texcoord.x/256.0+halfPix, a_texcoord.y/256.0+halfPix);
    color = a_color.abgr; // its reversed for some reason

    gl_Position = transform * model * position;
}

