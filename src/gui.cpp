#include "gui.h"

#include "imgui.h"

void DrawShadowDebugUi(ShadowSettings& settings, const float* cascadeSplits, int activeCascadeCount)
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowSize(ImVec2(360, 240), ImGuiCond_FirstUseEver);
    ImGui::Begin("Shadow Debug");
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

    ImGui::SliderFloat("Bias slope", &settings.shadowBiasSlope, 0.0f, 0.05f, "%.5f");
    ImGui::SliderFloat("Bias min", &settings.shadowBiasMin, 0.0f, 0.01f, "%.5f");

    ImGui::Checkbox("Use PCF", &settings.usePCF);

    ImGui::SeparatorText("CSM");
    ImGui::Checkbox("Use CSM", &settings.useCSM);

    const char* cascadeModes[] = { "3 Cascades", "5 Cascades", "7 Cascades" };
    int cascadeModeIndex = settings.cascadeCount == 3 ? 0 : settings.cascadeCount == 5 ? 1 : 2;

    ImGui::BeginDisabled(!settings.useCSM);
    if (ImGui::Combo("Cascade count", &cascadeModeIndex, cascadeModes, IM_ARRAYSIZE(cascadeModes)))
    {
        settings.cascadeCount = cascadeModeIndex == 0 ? 3 : cascadeModeIndex == 1 ? 5 : 7;
    }

    ImGui::SliderFloat("Split lambda", &settings.splitLambda, 0.0f, 1.0f);
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
