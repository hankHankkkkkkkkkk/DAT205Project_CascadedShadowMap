#ifndef GUI_H
#define GUI_H

enum class SceneMode
{
    Original = 0,
    CsmDemo = 1
};

enum class SunPreset
{
    Overhead = 0,
    FortyFiveDegrees = 1,
    FifteenDegrees = 2
};

struct SunSettings
{
    bool autoMove = false;
    bool movingForward = true;

    float pathAngleDegrees = 45.0f;
    float pathSpeed = 12.0f;
    float orbitRadius = 35.0f;
};

struct ShadowSettings
{
    SceneMode sceneMode = SceneMode::CsmDemo;
    SunSettings sun;

    bool usePCF = true;
    bool useCSM = true;
    bool showCascadeDebug = false;
    bool showDepthDebug = false;

    int cascadeCount = 3;

    float cameraNear = 0.1f;
    float cameraFar = 150.0f;
    float splitLambda = 0.5f;
    float cascadePadding = 10.0f;

    float shadowBiasSlope = 2.0f;
    float shadowBiasMin = 0.5f;
    float shadowCasterOffsetFactor = 0.1f;
    float shadowCasterOffsetUnits = 0.25f;
};

struct FrameStats;

void DrawShadowDebugUi(
    ShadowSettings& settings,
    const float* cascadeSplits,
    int activeCascadeCount,
    const FrameStats& frameStats
);

#endif
