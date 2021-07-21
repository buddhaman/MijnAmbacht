
#version 330 core

in vec3 normal;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
    vec3 lightDir = normalize(vec3(-1.0, -2.0, -3.0));
    float lightFactor = clamp(-dot(lightDir, normal), 0, 1);
    lightFactor = 0.6+lightFactor*0.4;
    vec4 texColor = texture(texture0, texCoord);
    vec3 color = lightFactor*texColor.xyz;
    FragColor = vec4(color, 1.0);
}

