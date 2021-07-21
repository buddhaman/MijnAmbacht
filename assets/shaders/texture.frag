#version 330 core

in vec2 textureCoords;
in vec3 normal;
in vec4 color;

out vec4 FragColor;
uniform sampler2D texture0;

void main()
{
    float diffuse = 0.4;
    vec4 texColor = texture(texture0, textureCoords);
    if(texColor.a < 0.1)
    {
        discard;
    }
    vec4 col = texColor*color;
    vec3 lightDir = vec3(-1, 1, -1);
    lightDir = normalize(lightDir);
    float dp = clamp(-dot(normal, lightDir), 0.0, 1.0);
    float lightFactor = diffuse * (1.0-dp) + dp;
    //lightFactor = 1.0;
    FragColor = vec4(lightFactor*col.xyz, col.a);
}

