#version 330 core

in vec2 texCoord;
in vec4 color;

out vec4 FragColor;

uniform sampler2D texture0;
uniform float radius;
uniform vec2 dir2;

// TODO: make uniform width and height
const float offset = 1.0/512.0;

void main()
{
    vec4 sum = vec4(0);
    float factor = 1.0/16.0;

    float hstep = dir2.x*offset*radius/4.0;
    float vstep = dir2.y*offset*radius/4.0;

    sum += texture2D(texture0, vec2(texCoord.x - 4.0*hstep, texCoord.y - 4.0*vstep)) * 0.0162162162;
	sum += texture2D(texture0, vec2(texCoord.x - 3.0*hstep, texCoord.y - 3.0*vstep)) * 0.0540540541;
	sum += texture2D(texture0, vec2(texCoord.x - 2.0*hstep, texCoord.y - 2.0*vstep)) * 0.1216216216;
	sum += texture2D(texture0, vec2(texCoord.x - 1.0*hstep, texCoord.y - 1.0*vstep)) * 0.1945945946;
	
	sum += texture2D(texture0, vec2(texCoord.x, texCoord.y)) * 0.2270270270;
	
	sum += texture2D(texture0, vec2(texCoord.x + 1.0*hstep, texCoord.y + 1.0*vstep)) * 0.1945945946;
	sum += texture2D(texture0, vec2(texCoord.x + 2.0*hstep, texCoord.y + 2.0*vstep)) * 0.1216216216;
	sum += texture2D(texture0, vec2(texCoord.x + 3.0*hstep, texCoord.y + 3.0*vstep)) * 0.0540540541;
	sum += texture2D(texture0, vec2(texCoord.x + 4.0*hstep, texCoord.y + 4.0*vstep)) * 0.0162162162;

    FragColor = color*sum;
}

