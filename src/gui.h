#ifndef GUI_H
#define GUI_H

enum class SceneMode
{
    Original = 0,
    CsmDemo = 1,
    Glass = 2
};

enum class SunPreset
{
    Overhead = 0,
    FortyFiveDegrees = 1,
    FifteenDegrees = 2
};

enum class ShadowTechnique
{
    Depth = 0,
    ColoredStochastic = 1,
    Deep = 2
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
    ShadowTechnique shadowTechnique = ShadowTechnique::Depth;

    int cascadeCount = 3;
    int singleShadowResolution = 2048;

    float cameraNear = 0.1f;
    float cameraFar = 150.0f;
    float splitLambda = 0.5f;
    float cascadePadding = 10.0f;

    float shadowBiasSlope = 2.0f;
    float shadowBiasMin = 0.5f;
    float shadowCasterOffsetFactor = 0.1f;
    float shadowCasterOffsetUnits = 0.25f;

    // Glass scene material opacity used by the transparent receiver/caster preview.
    float glassAlpha = 0.45f;
    // Keep stochastic coverage stable by default; animation is useful for sampling demos.
    bool animateStochasticNoise = false;
    // Frame-varying seed for stochastic colored shadow samples.
    int stochasticFrameIndex = 0;
    // Number of transparent layers captured by the deep shadow map.
    int deepShadowLayerCount = 4;
};

struct FrameStats;

void DrawShadowDebugUi(
    ShadowSettings& settings
);

void DrawShadowInfoOverlay(
    const ShadowSettings& settings,
    const float* cascadeSplits,
    int activeCascadeCount,
    const FrameStats& frameStats,
    unsigned int shadowMapResolution
);

#endif
