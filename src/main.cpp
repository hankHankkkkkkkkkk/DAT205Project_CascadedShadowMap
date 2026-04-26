#include <glad/glad.h>                          // Load OpenGL function pointers
#include <GLFW/glfw3.h>                         // Window creation and input handling
#include <iostream>

#include <glm/glm.hpp>                         // Core GLM functionality
#include <glm/gtc/matrix_transform.hpp>        // Matrix transformation helpers
#include <glm/gtc/type_ptr.hpp>                // Convert GLM types to raw pointer data

#include <sstream>      // Implement FPS/Frame time calculations
#include <iomanip>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "shader.h"

// Update the viewport whenever the framebuffer size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Handle keyboard input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "DAT205 Lit 3D Scene", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make this window's OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
    glViewport(0, 0, 800, 600);

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

    //glm::mat4 lightProjection = glm::ortho(
    //    -lightOrthoSize,
    //    lightOrthoSize,
    //    -lightOrthoSize,
    //    lightOrthoSize,
    //    lightNearPlane,
    //    lightFarPlane
    //);

    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Check whether the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Shadow framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        
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

		// light matrix setup for shadow mapping
        //float near_plane = 1.0f, far_plane = 15.0f;
        //glm::mat4 lightProjection = glm::ortho(-6.0f, 6.0f, -6.0f, 6.0f, near_plane, far_plane);
        
        if (lightFarPlane <= lightNearPlane + 0.1f)
        {
            lightFarPlane = lightNearPlane + 0.1f;
        }

        glm::mat4 lightProjection = glm::ortho(
            -lightOrthoSize,
            lightOrthoSize,
            -lightOrthoSize,
            lightOrthoSize,
            lightNearPlane,
            lightFarPlane
        );

        glm::vec3 lightPos = -lightDirection * 5.0f;
        glm::mat4 lightView = glm::lookAt(
            lightPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // 1. render scene to depth map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        renderScene(depthShader, planeVAO, cubeVAO);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        glViewport(0, 0, 800, 600);

        // Clear the screen and depth buffer every frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        shader.setBool("usePCF", usePCF);

        // Build a fixed camera view matrix
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, -1.2f, -6.0f));
        view = glm::rotate(view, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Build a perspective projection matrix
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f
        );

        // Send shared uniforms to the shader
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("lightSpaceMatrix", glm::value_ptr(lightSpaceMatrix));
        shader.setVec3("lightDirection", lightDirection.x, lightDirection.y, lightDirection.z);
        shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

		// Shadow bias parameters 
        shader.setFloat("shadowBiasSlope", shadowBiasSlope);
        shader.setFloat("shadowBiasMin", shadowBiasMin);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shader.setInt("shadowMap", 0);

		renderScene(shader, planeVAO, cubeVAO);

        //glBindVertexArray(cubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //Draw overlay
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("Shadow Debug");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

        // bias parameters
        ImGui::SliderFloat("Bias slope", &shadowBiasSlope, 0.0f, 0.05f, "%.5f");
        ImGui::SliderFloat("Bias min", &shadowBiasMin, 0.0f, 0.01f, "%.5f");

		// light frustum parameters
        ImGui::SliderFloat("Ortho size", &lightOrthoSize, 1.0f, 30.0f);
        ImGui::SliderFloat("Near plane", &lightNearPlane, 0.01f, 20.0f);
        ImGui::SliderFloat("Far plane", &lightFarPlane, 1.0f, 50.0f);

        // PCF toggle
        ImGui::Checkbox("Use PCF", &usePCF);

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
    glDeleteTextures(1, &depthMap);

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
