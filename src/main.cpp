#include <glad/glad.h>                          // Load OpenGL function pointers
#include <GLFW/glfw3.h>                         // Window creation and input handling
#include <iostream>

#include <glm/glm.hpp>                         // Core GLM functionality
#include <glm/gtc/matrix_transform.hpp>        // Matrix transformation helpers
#include <glm/gtc/type_ptr.hpp>                // Convert GLM types to raw pointer data

#include <sstream>      // Implement FPS/Frame time calculations
#include <iomanip>

#include <vector>
#include <cmath>
#include <algorithm>

#include <limits>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "shader.h"

// Initialize default window size 1080p
const unsigned int WINDOW_WIDTH = 1920;
const unsigned int WINDOW_HEIGHT = 1080;

// Initial camera height
const float CAMERA_FIXED_HEIGHT = 1.0f;

// Define maximum number of cascades
const int MAX_CASCADES = 7;


// Update the viewport whenever the framebuffer size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Implement movable camera
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 6.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.35f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = -20.0f;
float fov = 45.0f;

float lastMouseX = WINDOW_WIDTH / 2.0f;
float lastMouseY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

bool mouseLookEnabled = true;
bool spaceWasPressed = false;


// Handle keyboard input
void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceWasPressed)
    {
        mouseLookEnabled = !mouseLookEnabled;
        spaceWasPressed = true;
        firstMouse = true;

        if (mouseLookEnabled)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        spaceWasPressed = false;
    }

	// ensure camera stays at a fixed height (y = 3.0f) and only moves horizontally
    float cameraSpeed = 4.0f * deltaTime;

    glm::vec3 horizontalFront = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    glm::vec3 horizontalRight = glm::normalize(glm::cross(horizontalFront, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraSpeed * horizontalRight;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraSpeed * horizontalRight;

    //if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    //    cameraPos -= cameraSpeed * cameraUp;

    //if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    //    cameraPos += cameraSpeed * cameraUp;

	cameraPos.y = CAMERA_FIXED_HEIGHT;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!mouseLookEnabled)
    {
        return;
    }
    
    if (firstMouse)
    {
        lastMouseX = static_cast<float>(xpos);
        lastMouseY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastMouseX;
    float yoffset = lastMouseY - static_cast<float>(ypos);

    lastMouseX = static_cast<float>(xpos);
    lastMouseY = static_cast<float>(ypos);

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view);
glm::mat4 GetLightSpaceMatrix(float nearPlane, float farPlane, float aspect, const glm::vec3& lightDirection, float padding);
void UpdateCascadeSplits(float* cascadeSplits, int cascadeCount, float nearPlane, float farPlane, float lambda);

void renderScene(Shader& shader, unsigned int planeVAO, unsigned int cubeVAO);

int main()
{
    bool usePCF = true;
    
    double lastFrameTime = glfwGetTime();
    double fpsTimer = 0.0;
    int frameCount = 0;

    double currentFps = 0.0;
    double currentFrameTimeMs = 0.0;

    // Initialize GLFW
    glfwInit();

    // Request OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the application window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "DAT205 Lit 3D Scene", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make this window's OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize ImGui context and bind it to our GLFW window and OpenGL context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Set the initial viewport
    //glViewport(0, 0, 800, 600);

    // Enable depth testing so closer fragments overwrite farther ones
    glEnable(GL_DEPTH_TEST);

    // Load the lighting shader program
    Shader shader("assets/shaders/basic3d.vert", "assets/shaders/basic3d.frag");

    Shader depthShader("assets/shaders/depth.vert", "assets/shaders/depth.frag");

    // Cube vertex data:
    // Each vertex contains 6 floats:
    // position (x, y, z) + normal (nx, ny, nz)
    float cubeVertices[] = {
        // back face
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        // front face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // left face
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

        // right face
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

         // bottom face
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

         // top face
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f
    };

    // Plane vertex data:
    // position + upward normal
    float planeVertices[] = {
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,

         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        -5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f
    };

    // Create and fill the cube VAO/VBO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(
        0,                  // location
        3,                  // x, y, z
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),  // stride: 3 position + 3 normal
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(
        1,                              // location
        3,                              // nx, ny, nz
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))      // offset after position
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Create and fill the plane VAO/VBO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Define a directional light.
    // This vector points in the direction the light travels.
    glm::vec3 lightDirection(-0.5f, -1.0f, -0.3f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	// Define shadow bias parameters
    float shadowBiasSlope = 0.01f;
    float shadowBiasMin = 0.0015f;

	// Define light frustum parameters for shadow mapping
    float lightOrthoSize = 6.0f;
    float lightNearPlane = 1.0f;
    float lightFarPlane = 15.0f;

    // CSM parameters
    int cascadeCount = 3;
    float cameraNear = 0.1f;
    float cameraFar = 100.0f;
    float splitLambda = 0.5f;
    float cascadePadding = 10.0f;
    bool useCSM = true;
    bool showCascadeDebug = false;
    bool showDepthDebug = false;
    bool reportedCascadeFboError[MAX_CASCADES] = {};

    float cascadeSplits[MAX_CASCADES];
    glm::mat4 cascadeLightSpaceMatrices[MAX_CASCADES];

    //glm::mat4 lightProjection = glm::ortho(
    //    -lightOrthoSize,
    //    lightOrthoSize,
    //    -lightOrthoSize,
    //    lightOrthoSize,
    //    lightNearPlane,
    //    lightFarPlane
    //);

    const unsigned int SHADOW_WIDTH = 2048;
    const unsigned int SHADOW_HEIGHT = 2048;

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    //unsigned int depthMap;
    //glGenTextures(1, &depthMap);
    //glBindTexture(GL_TEXTURE_2D, depthMap);
    //glTexImage2D(
    //    GL_TEXTURE_2D,
    //    0,
    //    GL_DEPTH_COMPONENT,
    //    SHADOW_WIDTH,
    //    SHADOW_HEIGHT,
    //    0,
    //    GL_DEPTH_COMPONENT,
    //    GL_FLOAT,
    //    nullptr
    //);

    unsigned int depthMapArray;
    glGenTextures(1, &depthMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        MAX_CASCADES,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );


    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        //processInput(window);
        
        // Calculate frame time and FPS
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
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

        processInput(window, static_cast<float>(deltaTime));

		// light matrix setup for shadow mapping
        //float near_plane = 1.0f, far_plane = 15.0f;
        //glm::mat4 lightProjection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, near_plane, far_plane);
        
        if (lightFarPlane <= lightNearPlane + 0.1f)
        {
            lightFarPlane = lightNearPlane + 0.1f;
        }

        //glm::mat4 lightProjection = glm::ortho(
        //    -lightOrthoSize,
        //    lightOrthoSize,
        //    -lightOrthoSize,
        //    lightOrthoSize,
        //    lightNearPlane,
        //    lightFarPlane
        //);

        //glm::vec3 lightPos = -lightDirection * 5.0f;
        //glm::mat4 lightView = glm::lookAt(
        //    lightPos,
        //    glm::vec3(0.0f, 0.0f, 0.0f),
        //    glm::vec3(0.0f, 1.0f, 0.0f)
        //);

        //glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        int activeCascadeCount = useCSM ? cascadeCount : 1;
        UpdateCascadeSplits(cascadeSplits, activeCascadeCount, cameraNear, cameraFar, splitLambda);

        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        float aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
        
        float cascadeNear = cameraNear;
        for (int i = 0; i < activeCascadeCount; ++i)
        {
            float cascadeFar = cascadeSplits[i];

            cascadeLightSpaceMatrices[i] = GetLightSpaceMatrix(
                cascadeNear,
                cascadeFar,
                aspect,
                lightDirection,
                cascadePadding
            );

            cascadeNear = cascadeFar;
        } 
        
        // 1. render scene to depth map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        //depthShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        //renderScene(depthShader, planeVAO, cubeVAO);

		// Shadow map generation for each cascade
        for (int i = 0; i < activeCascadeCount; ++i)
        {
            glFramebufferTextureLayer(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                depthMapArray,
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
            renderScene(depthShader, planeVAO, cubeVAO);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        //glViewport(0, 0, 800, 600);

        // render the scence adaptively
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        // Clear the screen and depth buffer every frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setBool("usePCF", usePCF);

        // Build a fixed camera view matrix
        //glm::mat4 view = glm::mat4(1.0f);
        //view = glm::translate(view, glm::vec3(0.0f, -1.2f, -6.0f));
        //view = glm::rotate(view, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        //// Build a perspective projection matrix
        //glm::mat4 projection = glm::perspective(
        //    glm::radians(45.0f),
        //    800.0f / 600.0f,
        //    0.1f,
        //    100.0f
        //);

        //glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 6.0f);
        //glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        //glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            aspect,
            0.1f,
            100.0f
        );


        // Send shared uniforms to the shader
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        //shader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        shader.setVec3("lightDirection", lightDirection.x, lightDirection.y, lightDirection.z);
        shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

        shader.setInt("cascadeCount", activeCascadeCount);
        shader.setBool("showCascadeDebug", showCascadeDebug);
        shader.setBool("showDepthDebug", showDepthDebug);

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
        
        // Shadow bias parameters 
        shader.setFloat("shadowBiasSlope", shadowBiasSlope);
        shader.setFloat("shadowBiasMin", shadowBiasMin);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);
        shader.setInt("shadowMapArray", 0);

		renderScene(shader, planeVAO, cubeVAO);

        //glBindVertexArray(cubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //Draw overlay
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowSize(ImVec2(360, 240), ImGuiCond_FirstUseEver);
        ImGui::Begin("Shadow Debug");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

        // bias parameters
        ImGui::SliderFloat("Bias slope", &shadowBiasSlope, 0.0f, 0.05f, "%.5f");
        ImGui::SliderFloat("Bias min", &shadowBiasMin, 0.0f, 0.01f, "%.5f");

		// light frustum parameters
        //ImGui::SliderFloat("Ortho size", &lightOrthoSize, 1.0f, 30.0f);
        //ImGui::SliderFloat("Near plane", &lightNearPlane, 0.01f, 20.0f);
        //ImGui::SliderFloat("Far plane", &lightFarPlane, 1.0f, 50.0f);

        // PCF toggle
        ImGui::Checkbox("Use PCF", &usePCF);

		// CSM Debug parameters
        ImGui::SeparatorText("CSM");
        ImGui::Checkbox("Use CSM", &useCSM);

        const char* cascadeModes[] = { "3 Cascades", "5 Cascades", "7 Cascades" };
        int cascadeModeIndex = cascadeCount == 3 ? 0 : cascadeCount == 5 ? 1 : 2;

        ImGui::BeginDisabled(!useCSM);
        if (ImGui::Combo("Cascade count", &cascadeModeIndex, cascadeModes, IM_ARRAYSIZE(cascadeModes)))
        {
            cascadeCount = cascadeModeIndex == 0 ? 3 : cascadeModeIndex == 1 ? 5 : 7;
        }

        ImGui::SliderFloat("Split lambda", &splitLambda, 0.0f, 1.0f);
        ImGui::EndDisabled();

        ImGui::SliderFloat("Cascade padding", &cascadePadding, 0.0f, 50.0f);
        ImGui::Checkbox("Show cascade debug", &showCascadeDebug);
        ImGui::Checkbox("Show depth debug", &showDepthDebug);

        for (int i = 0; i < activeCascadeCount; ++i)
        {
            ImGui::Text("Cascade %d end: %.2f", i, cascadeSplits[i]);
        }
        
        ImGui::End();

		// Ensure far plane is always greater than near plane
        if (lightFarPlane <= lightNearPlane + 0.1f)
        {
            lightFarPlane = lightNearPlane + 0.1f;
        }

        //DrawShadowDebugOverlay(...);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //// Present the rendered frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Release GPU resources
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMapArray);

	// Clean up ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Clean up GLFW resources
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void renderScene(Shader& shader, unsigned int planeVAO, unsigned int cubeVAO)
{
    // plane
    glm::mat4 planeModel = glm::mat4(1.0f);
    shader.setMat4("model", glm::value_ptr(planeModel));
    shader.setVec3("objectColor", 0.7f, 0.7f, 0.7f);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // cube 1
    glm::mat4 cubeModel1 = glm::mat4(1.0f);
    cubeModel1 = glm::translate(cubeModel1, glm::vec3(-1.0f, 0.5f, -1.0f));
    shader.setMat4("model", glm::value_ptr(cubeModel1));
    shader.setVec3("objectColor", 1.0f, 0.5f, 0.2f);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // cube 2
    glm::mat4 cubeModel2 = glm::mat4(1.0f);
    cubeModel2 = glm::translate(cubeModel2, glm::vec3(1.2f, 0.5f, 0.8f));
    cubeModel2 = glm::scale(cubeModel2, glm::vec3(1.0f, 1.5f, 1.0f));
    shader.setMat4("model", glm::value_ptr(cubeModel2));
    shader.setVec3("objectColor", 0.2f, 0.6f, 1.0f);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void UpdateCascadeSplits(float* cascadeSplits, int cascadeCount, float nearPlane, float farPlane, float lambda)
{
    for (int i = 0; i < cascadeCount; ++i)
    {
        float p = static_cast<float>(i + 1) / static_cast<float>(cascadeCount);
        float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
        float linearSplit = nearPlane + (farPlane - nearPlane) * p;
        cascadeSplits[i] = lambda * logSplit + (1.0f - lambda) * linearSplit;
    }
}

std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view)
{
    glm::mat4 inverse = glm::inverse(projection * view);
    std::vector<glm::vec4> corners;

    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z)
            {
                glm::vec4 point = inverse * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f
                );

                corners.push_back(point / point.w);
            }
        }
    }

    return corners;
}

glm::mat4 GetLightSpaceMatrix(float nearPlane, float farPlane, float aspect, const glm::vec3& lightDirection, float padding)
{
    glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    std::vector<glm::vec4> corners = GetFrustumCornersWorldSpace(projection, view);

    glm::vec3 center(0.0f);
    for (const glm::vec4& corner : corners)
    {
        center += glm::vec3(corner);
    }
    center /= static_cast<float>(corners.size());

    glm::vec3 lightDir = glm::normalize(lightDirection);
    glm::mat4 lightView = glm::lookAt(
        center - lightDir * 30.0f,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const glm::vec4& corner : corners)
    {
        glm::vec4 trf = lightView * corner;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    float lightNearPlane = std::max(0.01f, -maxZ - padding);
    float lightFarPlane = std::max(lightNearPlane + 0.01f, -minZ + padding);

    return glm::ortho(
        minX - padding,
        maxX + padding,
        minY - padding,
        maxY + padding,
        lightNearPlane,
        lightFarPlane
    ) * lightView;

}
