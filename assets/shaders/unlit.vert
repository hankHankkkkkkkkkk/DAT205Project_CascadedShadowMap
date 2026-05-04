#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Unlit marker shader: draw the visible sun without applying scene lighting.
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
