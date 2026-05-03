#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
// out vec4 FragPosLightSpace;

out vec4 FragPosViewSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform mat4 lightSpaceMatrix;

void main()
{
    // Compute the world-space position of the vertex
    vec4 worldPosition = model * vec4(aPos, 1.0);
    FragPos = worldPosition.xyz;

    // Transform the normal into world space
    Normal = mat3(transpose(inverse(model))) * aNormal;
    // FragPosLightSpace = lightSpaceMatrix * worldPosition;

    FragPosViewSpace = view * worldPosition;

    // Standard MVP transform
    gl_Position = projection * view * worldPosition;
}