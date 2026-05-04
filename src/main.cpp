#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    Mesh cube = CreateCubeMesh();
    Mesh plane = CreatePlaneMesh();
    ShadowMap shadowMap = CreateShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT, MAX_CASCADES);

    glm::vec3 lightDirection(-0.5f, -1.0f, -0.3f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    ShadowSettings shadowSettings;
    bool reportedCascadeFboError[MAX_CASCADES] = {};

    float cascadeSplits[MAX_CASCADES] = {};
    glm::mat4 cascadeLightSpaceMatrices[MAX_CASCADES] = {};

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

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        const float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);

        const int activeCascadeCount = shadowSettings.useCSM ? shadowSettings.cascadeCount : 1;
        UpdateCascadeSplits(
            cascadeSplits,
            activeCascadeCount,
            shadowSettings.cameraNear,
            shadowSettings.cameraFar,
            shadowSettings.splitLambda
        );

        float cascadeNear = shadowSettings.cameraNear;
        for (int i = 0; i < activeCascadeCount; ++i)
        {
            const float cascadeFar = cascadeSplits[i];

            cascadeLightSpaceMatrices[i] = GetLightSpaceMatrix(
                cascadeNear,
                cascadeFar,
                aspect,
                camera.zoom(),
                camera.position(),
                camera.front(),
                camera.up(),
                lightDirection,
                shadowSettings.cascadePadding
            );

            cascadeNear = cascadeFar;
        }

        glViewport(0, 0, shadowMap.width, shadowMap.height);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);

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
            RenderScene(depthShader, plane, cube);
        }

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
            shadowSettings.cameraNear,
            shadowSettings.cameraFar
        );

        shader.setBool("usePCF", shadowSettings.usePCF);
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("lightDirection", lightDirection.x, lightDirection.y, lightDirection.z);
        shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

        shader.setInt("cascadeCount", activeCascadeCount);
        shader.setBool("showCascadeDebug", shadowSettings.showCascadeDebug);
        shader.setBool("showDepthDebug", shadowSettings.showDepthDebug);

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
        }

        shader.setFloat("shadowBiasSlope", shadowSettings.shadowBiasSlope);
        shader.setFloat("shadowBiasMin", shadowSettings.shadowBiasMin);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap.textureArray);
        shader.setInt("shadowMapArray", 0);

        RenderScene(shader, plane, cube);

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
    plane.destroy();
    shadowMap.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
