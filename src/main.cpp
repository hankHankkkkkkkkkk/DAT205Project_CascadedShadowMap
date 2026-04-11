#include <glad/glad.h>                          // Load OpenGL function pointers
#include <GLFW/glfw3.h>                         // Window creation and input handling
#include <iostream>

#include <glm/glm.hpp>                         // Core GLM functionality
#include <glm/gtc/matrix_transform.hpp>        // Matrix transformation helpers
#include <glm/gtc/type_ptr.hpp>                // Convert GLM types to raw pointer data

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

int main()
{
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

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Clear the screen and depth buffer every frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

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
        shader.setVec3("lightDirection", lightDirection.x, lightDirection.y, lightDirection.z);
        shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

        // -------- Draw the ground plane --------
        glm::mat4 planeModel = glm::mat4(1.0f);
        shader.setMat4("model", glm::value_ptr(planeModel));
        shader.setVec3("objectColor", 0.7f, 0.7f, 0.7f);

        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // -------- Draw cube 1 --------
        glm::mat4 cubeModel1 = glm::mat4(1.0f);
        cubeModel1 = glm::translate(cubeModel1, glm::vec3(-1.0f, 0.5f, -1.0f));
        shader.setMat4("model", glm::value_ptr(cubeModel1));
        shader.setVec3("objectColor", 1.0f, 0.5f, 0.2f);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -------- Draw cube 2 --------
        glm::mat4 cubeModel2 = glm::mat4(1.0f);
        cubeModel2 = glm::translate(cubeModel2, glm::vec3(1.2f, 0.5f, 0.8f));
        cubeModel2 = glm::scale(cubeModel2, glm::vec3(1.0f, 1.5f, 1.0f));
        shader.setMat4("model", glm::value_ptr(cubeModel2));
        shader.setVec3("objectColor", 0.2f, 0.6f, 1.0f);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Present the rendered frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Release GPU resources
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    // Clean up GLFW resources
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}