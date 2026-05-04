#version 330 core

out vec4 FragColor;

uniform vec3 color;

void main()
{
    // Unlit marker color: keep the sun visible regardless of shadowing.
    FragColor = vec4(color, 1.0);
}
