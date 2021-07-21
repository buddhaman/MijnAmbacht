
#version 330 core

in vec3 normal;
out vec4 FragColor;

void main()
{
    vec3 lightDir = normalize(vec3(-1.0, -2.0, -3.0));
    float lightFactor = clamp(-dot(lightDir, normal), 0, 1);
    lightFactor = 0.6+lightFactor*0.4;
    vec3 color = lightFactor*vec3(1.0, 1.0, 1.0);
    FragColor = vec4(color, 1.0);
}

