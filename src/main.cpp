#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include "camera.h"
#include "config.h"
#include "csm.h"
#include "gui.h"
#include "mesh.h"
#include "render.h"
#include "shader.h"
#include "shadowMap.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

ShadowSettings GetEffectiveShadowSettings(const ShadowSettings& uiSettings)
{
    // Effective settings keep UI-controlled debug values while applying scene-specific camera ranges.
    ShadowSettings effectiveSettings = uiSettings;

    if (uiSettings.sceneMode == SceneMode::Original)
    {
        // Original scene keeps the smaller historical camera range, but debug overlay values
        // such as bias, CSM mode, cascade count, padding, and PCF remain interactive.
        effectiveSettings.cameraNear = 0.1f;
        effectiveSettings.cameraFar = 100.0f;
    }

    return effectiveSettings;
}

void UpdateSunMotion(SunSettings& sun, float deltaTime)
{
    // Auto motion bounces along a fixed solar arc to simulate sunrise-to-sunset movement.
    if (!sun.autoMove)
    {
        return;
    }

    const float direction = sun.movingForward ? 1.0f : -1.0f;
    sun.pathAngleDegrees += direction * sun.pathSpeed * deltaTime;

    if (sun.pathAngleDegrees >= 165.0f)
    {
        sun.pathAngleDegrees = 165.0f;
        sun.movingForward = false;
    }
    else if (sun.pathAngleDegrees <= 15.0f)
    {
        sun.pathAngleDegrees = 15.0f;
        sun.movingForward = true;
    }
}

glm::vec3 CalculateSunPosition(const SunSettings& sun)
{
    // Solar path: move the sun on a vertical east-west arc above the shared scene center.
    const float pathAngle = glm::radians(sun.pathAngleDegrees);
    const float radius = sun.orbitRadius;

    return glm::vec3(
        radius * glm::cos(pathAngle),
        radius * glm::sin(pathAngle),
        -radius * 0.35f
    );
}

int main()
{
    double lastFrameTime = glfwGetTime();
    double fpsTimer = 0.0;
    int frameCount = 0;

    double currentFps = 0.0;
    double currentFrameTimeMs = 0.0;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "DAT205 Lit 3D Scene", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    Camera camera;

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, Camera::mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("assets/shaders/basic3d.vert", "assets/shaders/basic3d.frag");
    Shader depthShader("assets/shaders/depth.vert", "assets/shaders/depth.frag");
    Shader unlitShader("assets/shaders/unlit.vert", "assets/shaders/unlit.frag");

    Mesh cube = CreateCubeMesh();
    Mesh smallPlane = CreateSmallPlaneMesh();
    Mesh largePlane = CreateLargePlaneMesh();
    ShadowMap shadowMap = CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT, MAX_CASCADES);

    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    ShadowSettings shadowSettings;
    bool reportedCascadeFboError[MAX_CASCADES] = {};

    float cascadeSplits[MAX_CASCADES] = {};
    glm::mat4 cascadeLightSpaceMatrices[MAX_CASCADES] = {};
    float cascadeLightDepthRanges[MAX_CASCADES] = {};
    float cascadeWorldTexelSizes[MAX_CASCADES] = {};
    const glm::vec3 sceneMin(-90.0f, -5.0f, -90.0f);
    const glm::vec3 sceneMax(90.0f, 30.0f, 90.0f);
    constexpr float cascadeOverlapRatio = 0.10f;
    constexpr float cascadeBlendRatio = 0.08f;

    while (!glfwWindowShouldClose(window))
    {
        const double currentFrameTime = glfwGetTime();
        const double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        fpsTimer += deltaTime;
        frameCount++;

        if (fpsTimer >= 0.25)
        {
            currentFps = frameCount / fpsTimer;
            currentFrameTimeMs = 1000.0 / currentFps;

            std::ostringstream title;
            title << "DAT205 Lit 3D Scene | FPS: "
                << std::fixed << std::setprecision(1) << currentFps
                << " | Frame: "
                << std::fixed << std::setprecision(2) << currentFrameTimeMs
                << " ms";

            glfwSetWindowTitle(window, title.str().c_str());

            frameCount = 0;
            fpsTimer = 0.0;
        }

        camera.processInput(window, static_cast<float>(deltaTime));
        UpdateSunMotion(shadowSettings.sun, static_cast<float>(deltaTime));

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        const float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);

        const ShadowSettings effectiveSettings = GetEffectiveShadowSettings(shadowSettings);

        // Both scenes share one sun marker and one directional light derived from that marker.
        const glm::vec3 sunOrbitCenter(0.0f, 0.0f, -10.0f);
        const glm::vec3 sunPosition = sunOrbitCenter + CalculateSunPosition(effectiveSettings.sun);
        const glm::vec3 lightDirection = glm::normalize(sunOrbitCenter - sunPosition);

        const int activeCascadeCount = effectiveSettings.useCSM ? effectiveSettings.cascadeCount : 1;
        // Single-layer shadow maps use a shorter range so the non-CSM comparison remains readable.
        const float shadowFarPlane = effectiveSettings.useCSM ? effectiveSettings.cameraFar : 45.0f;

        UpdateCascadeSplits(
            cascadeSplits,
            activeCascadeCount,
            effectiveSettings.cameraNear,
            shadowFarPlane,
            effectiveSettings.splitLambda
        );


        float cascadeNear = effectiveSettings.cameraNear;
        for (int i = 0; i < activeCascadeCount; ++i)
        {
            const float cascadeFar = cascadeSplits[i];
            const float cascadeLength = cascadeFar - cascadeNear;
            const float overlap = effectiveSettings.useCSM ? cascadeLength * cascadeOverlapRatio : 0.0f;
            const float renderNear = std::max(effectiveSettings.cameraNear, cascadeNear - overlap);
            const float renderFar = std::min(shadowFarPlane, cascadeFar + overlap);

            cascadeLightSpaceMatrices[i] = GetLightSpaceMatrix(
                renderNear,
                renderFar,
                aspect,
                camera.zoom(),
                camera.position(),
                camera.front(),
                camera.up(),
                lightDirection,
                effectiveSettings.cascadePadding,
                sceneMin,
                sceneMax,
                shadowMap.width,
                &cascadeLightDepthRanges[i],
                &cascadeWorldTexelSizes[i]
            );

            cascadeNear = cascadeFar;
        }

        glViewport(0, 0, shadowMap.width, shadowMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(
            effectiveSettings.shadowCasterOffsetFactor,
            effectiveSettings.shadowCasterOffsetUnits
        );

        depthShader.use();
        for (int i = 0; i < activeCascadeCount; ++i)
        {
            glFramebufferTextureLayer(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                shadowMap.textureArray,
                0,
                i
            );

            if (!reportedCascadeFboError[i] && glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cerr << "Cascade FBO incomplete at layer " << i << std::endl;
                reportedCascadeFboError[i] = true;
            }

            glClear(GL_DEPTH_BUFFER_BIT);

            depthShader.setMat4("lightSpaceMatrix", glm::value_ptr(cascadeLightSpaceMatrices[i]));
            RenderScene(depthShader, smallPlane, largePlane, cube, effectiveSettings.sceneMode);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        const glm::mat4 view = camera.getViewMatrix();
        const glm::mat4 projection = glm::perspective(
            glm::radians(camera.zoom()),
            aspect,
            effectiveSettings.cameraNear,
            effectiveSettings.cameraFar
        );

        shader.setBool("usePCF", effectiveSettings.usePCF);
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("lightDirection", lightDirection.x, lightDirection.y, lightDirection.z);
        shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

        shader.setInt("cascadeCount", activeCascadeCount);
        shader.setFloat("cascadeBlendRatio", effectiveSettings.useCSM ? cascadeBlendRatio : 0.0f);
        shader.setBool("showCascadeDebug", effectiveSettings.showCascadeDebug);
        shader.setBool("showDepthDebug", effectiveSettings.showDepthDebug);

        for (int i = 0; i < activeCascadeCount; ++i)
        {
            shader.setMat4(
                "lightSpaceMatrices[" + std::to_string(i) + "]",
                glm::value_ptr(cascadeLightSpaceMatrices[i])
            );

            shader.setFloat(
                "cascadeSplits[" + std::to_string(i) + "]",
                cascadeSplits[i]
            );

            shader.setFloat(
                "cascadeLightDepthRanges[" + std::to_string(i) + "]",
                cascadeLightDepthRanges[i]
            );

            shader.setFloat(
                "cascadeWorldTexelSizes[" + std::to_string(i) + "]",
                cascadeWorldTexelSizes[i]
            );
        }

        // Bias is scene-preset aware, with extra reduction for the single-layer comparison mode.
        const float effectiveBiasSlope = effectiveSettings.useCSM
            ? effectiveSettings.shadowBiasSlope
            : effectiveSettings.shadowBiasSlope * 0.1f;

        const float effectiveBiasMin = effectiveSettings.useCSM
            ? effectiveSettings.shadowBiasMin
            : effectiveSettings.shadowBiasMin * 0.05f;

        shader.setFloat("shadowBiasSlope", effectiveBiasSlope);
        shader.setFloat("shadowBiasMin", effectiveBiasMin);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap.textureArray);
        shader.setInt("shadowMapArray", 0);

        RenderScene(shader, smallPlane, largePlane, cube, effectiveSettings.sceneMode);

        unlitShader.use();
        unlitShader.setMat4("view", glm::value_ptr(view));
        unlitShader.setMat4("projection", glm::value_ptr(projection));

        // Visible sun marker: this shows the shared directional light source position.
        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, sunPosition);
        sunModel = glm::scale(sunModel, glm::vec3(0.7f));

        unlitShader.setMat4("model", glm::value_ptr(sunModel));
        unlitShader.setVec3("color", 1.0f, 0.86f, 0.35f);

        glBindVertexArray(cube.vao);
        glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawShadowDebugUi(shadowSettings, cascadeSplits, activeCascadeCount);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cube.destroy();
    smallPlane.destroy();
    largePlane.destroy();
    shadowMap.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
