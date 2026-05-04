#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    void processInput(GLFWwindow* window, float deltaTime);
    void processMouseMovement(double xpos, double ypos);

    glm::mat4 getViewMatrix() const;

    const glm::vec3& position() const;
    const glm::vec3& front() const;
    const glm::vec3& up() const;
    float zoom() const;

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

private:
    void updateFront();

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;

    float yaw_;
    float pitch_;
    float fov_;

    float lastMouseX_;
    float lastMouseY_;
    bool firstMouse_;
    bool mouseLookEnabled_;
    bool spaceWasPressed_;
};

#endif
