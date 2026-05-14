#include "gui.h"

#include "gpuTimer.h"

#include "imgui.h"

namespace
{
void ApplySunPreset(SunSettings& sun, SunPreset preset)
{
    // Sun presets snap the simulated solar path to useful inspection angles.
    switch (preset)
    {
    case SunPreset::Overhead:
        sun.pathAngleDegrees = 85.0f;
        break;

    case SunPreset::FortyFiveDegrees:
        sun.pathAngleDegrees = 45.0f;
        break;

    case SunPreset::FifteenDegrees:
    default:
        sun.pathAngleDegrees = 15.0f;
        break;
    }
}
}

void DrawShadowDebugUi(
    ShadowSettings& settings,
    const float* cascadeSplits,
    int activeCascadeCount,
    const FrameStats& frameStats,
    unsigned int shadowMapResolution)
{
    ImGui::SetNextWindowSize(ImVec2(360, 240), ImGuiCond_FirstUseEver);
    ImGui::Begin("Shadow Debug");

    if (frameStats.hasGpuTiming)
    {
        ImGui::Text("GPU FPS: %.1f", frameStats.gpuFps);
        ImGui::Text("GPU frame time: %.3f ms", frameStats.gpuFrameTimeMs);
    }
    else
    {
        ImGui::TextUnformatted("GPU FPS: warming up");
        ImGui::TextUnformatted("GPU frame time: warming up");
    }

    ImGui::SliderFloat("Bias slope texels", &settings.shadowBiasSlope, 0.0f, 8.0f, "%.2f");
    ImGui::SliderFloat("Bias min texels", &settings.shadowBiasMin, 0.0f, 4.0f, "%.2f");
    ImGui::SliderFloat("Caster offset factor", &settings.shadowCasterOffsetFactor, 0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Caster offset units", &settings.shadowCasterOffsetUnits, 0.0f, 2.0f, "%.2f");

    ImGui::Checkbox("Use PCF", &settings.usePCF);

    ImGui::SeparatorText("Sun");

    // Sun controls drive one shared directional light and its visible marker.
    ImGui::Checkbox("Auto move sun", &settings.sun.autoMove);
    ImGui::SliderFloat("Sun path angle", &settings.sun.pathAngleDegrees, 5.0f, 175.0f, "%.1f deg");
    ImGui::SliderFloat("Sun speed", &settings.sun.pathSpeed, 0.0f, 60.0f, "%.1f deg/s");
    ImGui::SliderFloat("Sun radius", &settings.sun.orbitRadius, 10.0f, 80.0f, "%.1f");

    // Preset buttons make common light directions quick to compare.
    if (ImGui::Button("Overhead"))
    {
        ApplySunPreset(settings.sun, SunPreset::Overhead);
    }
    ImGui::SameLine();
    if (ImGui::Button("45 deg"))
    {
        ApplySunPreset(settings.sun, SunPreset::FortyFiveDegrees);
    }
    ImGui::SameLine();
    if (ImGui::Button("15 deg"))
    {
        ApplySunPreset(settings.sun, SunPreset::FifteenDegrees);
    }

    ImGui::SeparatorText("Scene");

    // Scene selector: the original scene keeps its historical camera range.
    const char* sceneModes[] = { "Original", "CSM Demo" };
    int sceneModeIndex = static_cast<int>(settings.sceneMode);
    if (ImGui::Combo("Scene", &sceneModeIndex, sceneModes, IM_ARRAYSIZE(sceneModes)))
    {
        settings.sceneMode = static_cast<SceneMode>(sceneModeIndex);
    }

    if (settings.sceneMode == SceneMode::Original)
    {
        ImGui::TextWrapped("Original scene uses the preserved small-scene camera range.");
    }

    ImGui::SeparatorText("CSM");
    ImGui::Checkbox("Use CSM", &settings.useCSM);

    ImGui::Text(
        "Shadow map: %s %ux%u%s",
        settings.useCSM ? "CSM" : "Single",
        shadowMapResolution,
        shadowMapResolution,
        settings.useCSM ? " array" : ""
    );

    const char* cascadeModes[] = { "3 Cascades", "5 Cascades", "7 Cascades" };
    int cascadeModeIndex = settings.cascadeCount == 3 ? 0 : settings.cascadeCount == 5 ? 1 : 2;

    ImGui::BeginDisabled(!settings.useCSM);
    if (ImGui::Combo("Cascade count", &cascadeModeIndex, cascadeModes, IM_ARRAYSIZE(cascadeModes)))
    {
        settings.cascadeCount = cascadeModeIndex == 0 ? 3 : cascadeModeIndex == 1 ? 5 : 7;
    }

    ImGui::SliderFloat("Split lambda", &settings.splitLambda, 0.0f, 1.0f);
    ImGui::EndDisabled();

    const char* singleResolutionLabels[] = { "1024", "2048", "4096", "8192" };
    int singleResolutionIndex = settings.singleShadowResolution == 1024 ? 0
        : settings.singleShadowResolution == 2048 ? 1
        : settings.singleShadowResolution == 4096 ? 2
        : 3;

    ImGui::BeginDisabled(settings.useCSM);
    if (ImGui::Combo("Single resolution", &singleResolutionIndex, singleResolutionLabels, IM_ARRAYSIZE(singleResolutionLabels)))
    {
        const int singleResolutions[] = { 1024, 2048, 4096, 8192 };
        settings.singleShadowResolution = singleResolutions[singleResolutionIndex];
    }
    ImGui::EndDisabled();

    ImGui::SliderFloat("Cascade padding", &settings.cascadePadding, 0.0f, 50.0f);
    ImGui::Checkbox("Show cascade debug", &settings.showCascadeDebug);
    ImGui::Checkbox("Show depth debug", &settings.showDepthDebug);

    for (int i = 0; i < activeCascadeCount; ++i)
    {
        ImGui::Text("Cascade %d end: %.2f", i, cascadeSplits[i]);
    }

    ImGui::End();
}
