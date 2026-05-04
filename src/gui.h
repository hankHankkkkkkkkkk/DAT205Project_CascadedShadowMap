#ifndef GUI_H
#define GUI_H

enum class SceneMode
{
    Original = 0,
    CsmDemo = 1
};

struct ShadowSettings
{
    SceneMode sceneMode = SceneMode::CsmDemo;

    bool usePCF = true;
    bool useCSM = true;
    bool showCascadeDebug = false;
    bool showDepthDebug = false;

    int cascadeCount = 3;

    float cameraNear = 0.1f;
    float cameraFar = 150.0f;
    float splitLambda = 0.5f;
    float cascadePadding = 10.0f;

    float shadowBiasSlope = 0.01f;
    float shadowBiasMin = 0.0015f;
};

void DrawShadowDebugUi(ShadowSettings& settings, const float* cascadeSplits, int activeCascadeCount);

#endif
