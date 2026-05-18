#version 330 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform float objectAlpha;
uniform int stochasticFrameIndex;

float Hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

void main()
{
    // Alpha is converted into stochastic coverage so the depth map remains binary per sample.
    float coverage = Hash13(vec3(gl_FragCoord.xy, float(stochasticFrameIndex)));
    if (coverage > objectAlpha)
    {
        discard;
    }

    FragColor = vec4(objectColor, 1.0);
}
