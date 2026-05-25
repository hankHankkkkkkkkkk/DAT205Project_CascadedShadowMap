#version 330 core

layout (location = 0) out vec4 DeepColor;
layout (location = 1) out float DeepDepth;

uniform vec3 objectColor;
uniform float objectAlpha;
uniform sampler2DArray previousDeepDepthMap;
uniform int peelLayer;
uniform float peelDepthEpsilon;

void main()
{
    ivec2 depthSize = textureSize(previousDeepDepthMap, 0).xy;
    vec2 uv = gl_FragCoord.xy / vec2(depthSize);

    // Depth peeling keeps the nearest transparent surface that is still behind the previous layer.
    if (peelLayer > 0)
    {
        float previousDepth = texture(previousDeepDepthMap, vec3(uv, float(peelLayer - 1))).r;
        if (gl_FragCoord.z <= previousDepth + peelDepthEpsilon)
        {
            discard;
        }
    }

    DeepColor = vec4(objectColor, objectAlpha);
    DeepDepth = gl_FragCoord.z;
}
