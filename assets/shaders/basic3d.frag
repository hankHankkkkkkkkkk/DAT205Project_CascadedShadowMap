#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform from [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // outside the light frustum: do not apply shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 0.0;
    }

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // simple bias
    float bias = max(0.005 * (1.0 - dot(normalize(normal), normalize(-lightDir))), 0.0005);

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}



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
    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDirection);
    vec3 result = (ambient + (1.0 - shadow) * diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}
