#version 330 core

const int MAX_CASCADES = 7;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
// in vec4 FragPosLightSpace;

in vec4 FragPosViewSpace;

uniform vec3 objectColor;
uniform float objectAlpha;
uniform vec3 lightColor;
uniform vec3 lightDirection;
// uniform sampler2D shadowMap;

// Shadow bias parameters
uniform float shadowBiasSlope;
uniform float shadowBiasMin;

// Toggle for using PCF
uniform bool usePCF;

const int SHADOW_TECHNIQUE_DEPTH = 0;
const int SHADOW_TECHNIQUE_COLORED_STOCHASTIC = 1;
const int SHADOW_TECHNIQUE_DEEP = 2;

// Implement CSM
uniform sampler2DArray shadowMapArray;
uniform sampler2D stochasticShadowColorMap;
uniform sampler2D stochasticShadowDepthMap;
uniform mat4 lightSpaceMatrices[MAX_CASCADES];
uniform float cascadeSplits[MAX_CASCADES];
uniform float cascadeLightDepthRanges[MAX_CASCADES];
uniform float cascadeWorldTexelSizes[MAX_CASCADES];
uniform int cascadeCount;
uniform int shadowTechnique;
uniform float cascadeBlendRatio;
uniform bool showCascadeDebug;
uniform bool showDepthDebug;

vec3 depthDebugColor = vec3(0.0);

int SelectCascade(float viewDepth)
{
    int cascadeIndex = cascadeCount - 1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (viewDepth < cascadeSplits[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    return cascadeIndex;
}

float SampleShadowCascade(vec3 fragPos, vec3 normal, vec3 lightDir, int cascadeIndex)
{
    vec4 fragPosLightSpace = lightSpaceMatrices[cascadeIndex] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform from [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // outside the light frustum: do not apply shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        if (showDepthDebug)
        {
            if (projCoords.x < 0.0 || projCoords.x > 1.0)
            {
                depthDebugColor = vec3(1.0, 0.0, 0.0);
            }
            else if (projCoords.y < 0.0 || projCoords.y > 1.0)
            {
                depthDebugColor = vec3(1.0, 0.0, 1.0);
            }
            else
            {
                depthDebugColor = vec3(0.0, 0.4, 1.0);
            }
        }
        return 0.0;
    }

    float currentDepth = projCoords.z;

    float ndotl = dot(normalize(normal), normalize(-lightDir));
    float worldTexelSize = max(cascadeWorldTexelSizes[cascadeIndex], 0.000001);
    float lightDepthRange = max(cascadeLightDepthRanges[cascadeIndex], 0.000001);

    // Bias is authored in shadow texels, then converted to normalized light depth.
    // This keeps peter-panning stable when a cascade's caster depth range grows.
    float worldBias = max(
        shadowBiasSlope * worldTexelSize * (1.0 - ndotl),
        shadowBiasMin * worldTexelSize
    );
    worldBias = min(worldBias, 4.0 * worldTexelSize);
    float bias = worldBias / lightDepthRange;

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);
    float shadow = 0.0;

    if (!usePCF)
    {
        // float closestDepth = texture(shadowMap, projCoords.xy).r;
        
        float closestDepth = texture(shadowMapArray, vec3(projCoords.xy, float(cascadeIndex))).r;
        if (showDepthDebug)
        {
            depthDebugColor = closestDepth < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 1.0, 0.0);
            return 0.0;
        }
        return currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }


    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMapArray, vec3(projCoords.xy + vec2(x, y) * texelSize, float(cascadeIndex))).r;

            if (showDepthDebug)
            {
                shadow += pcfDepth < 0.999 ? 1.0 : 0.0;
                continue;
            }

            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    if (showDepthDebug)
    {
        depthDebugColor = shadow > 0.0 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 1.0, 0.0);
        return 0.0;
    }

    return shadow;
}

float ShadowCalculation(vec3 fragPos, float viewDepth, vec3 normal, vec3 lightDir, out int cascadeIndex)
{
    cascadeIndex = SelectCascade(viewDepth);
    float shadow = SampleShadowCascade(fragPos, normal, lightDir, cascadeIndex);

    if (cascadeBlendRatio <= 0.0 || cascadeIndex >= cascadeCount - 1 || showDepthDebug)
    {
        return shadow;
    }

    float cascadeStart = cascadeIndex == 0 ? 0.0 : cascadeSplits[cascadeIndex - 1];
    float cascadeEnd = cascadeSplits[cascadeIndex];
    float blendRange = max((cascadeEnd - cascadeStart) * cascadeBlendRatio, 0.001);
    float distanceToSplit = cascadeEnd - viewDepth;

    // Blend across split edges so adjacent valid cascades do not produce a visible seam.
    if (distanceToSplit > 0.0 && distanceToSplit < blendRange)
    {
        float nextShadow = SampleShadowCascade(fragPos, normal, lightDir, cascadeIndex + 1);
        float blend = 1.0 - distanceToSplit / blendRange;
        shadow = mix(shadow, nextShadow, blend);
    }

    return shadow;
}

vec3 SampleColoredStochasticTransmission(vec3 fragPos, vec3 normal, vec3 lightDir)
{
    vec4 fragPosLightSpace = lightSpaceMatrices[0] * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return vec3(1.0);
    }

    float ndotl = dot(normalize(normal), normalize(-lightDir));
    float worldTexelSize = max(cascadeWorldTexelSizes[0], 0.000001);
    float lightDepthRange = max(cascadeLightDepthRanges[0], 0.000001);

    // Use the same world-space bias model so colored shadows align with depth shadows.
    float worldBias = max(
        shadowBiasSlope * worldTexelSize * (1.0 - ndotl),
        shadowBiasMin * worldTexelSize
    );
    worldBias = min(worldBias, 4.0 * worldTexelSize);
    float bias = worldBias / lightDepthRange;

    vec2 texelSize = 1.0 / vec2(textureSize(stochasticShadowColorMap, 0));
    vec3 accumulatedColor = vec3(0.0);
    float hits = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 uv = projCoords.xy + vec2(x, y) * texelSize;
            float stochasticDepth = texture(stochasticShadowDepthMap, uv).r;
            vec4 stochasticColor = texture(stochasticShadowColorMap, uv);

            if (projCoords.z - bias > stochasticDepth && stochasticColor.a > 0.5)
            {
                accumulatedColor += stochasticColor.rgb;
                hits += 1.0;
            }
        }
    }

    if (hits <= 0.0)
    {
        return vec3(1.0);
    }

    float coverage = hits / 9.0;
    vec3 averageColor = accumulatedColor / hits;
    return mix(vec3(1.0), averageColor, coverage);
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
    //float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDirection);
    
    int cascadeIndex = 0;
    float viewDepth = abs(FragPosViewSpace.z);
    float shadow = ShadowCalculation(FragPos, viewDepth, norm, lightDirection, cascadeIndex);

    vec3 diffuseTransmission = vec3(1.0);
    if (shadowTechnique == SHADOW_TECHNIQUE_COLORED_STOCHASTIC)
    {
        diffuseTransmission = SampleColoredStochasticTransmission(FragPos, norm, lightDirection);
    }

    vec3 result = (ambient + (1.0 - shadow) * diffuse * diffuseTransmission) * objectColor;

    if (showDepthDebug)
    {
        FragColor = vec4(depthDebugColor, 1.0);
        return;
    }

    if (showCascadeDebug)
    {
        vec3 colors[7] = vec3[](
            vec3(1.0, 0.2, 0.2),
            vec3(0.2, 1.0, 0.2),
            vec3(0.2, 0.4, 1.0),
            vec3(1.0, 1.0, 0.2),
            vec3(1.0, 0.2, 1.0),
            vec3(0.2, 1.0, 1.0),
            vec3(1.0, 0.6, 0.2)
        );

        result = mix(result, colors[cascadeIndex], 0.35);
    }

    FragColor = vec4(result, objectAlpha);
}
