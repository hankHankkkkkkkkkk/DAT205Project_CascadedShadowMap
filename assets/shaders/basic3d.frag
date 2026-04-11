#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDirection;

void main()
{
    // Ambient lighting: a small constant light term
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // Normalize the input normal
    vec3 norm = normalize(Normal);

    // Convert light direction into the direction from fragment toward the light
    vec3 lightDir = normalize(-lightDirection);

    // Diffuse lighting based on Lambert's cosine law
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Final lighting result
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}