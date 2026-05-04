#include "camera.h"

#include "config.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

Camera::Camera()
    : position_(0.0f, 3.0f, 6.0f),
      front_(0.0f, -0.35f, -1.0f),
      up_(0.0f, 1.0f, 0.0f),
      yaw_(-90.0f),
      pitch_(-20.0f),
      fov_(45.0f),
      lastMouseX_(WINDOW_WIDTH / 2.0f),
      lastMouseY_(WINDOW_HEIGHT / 2.0f),
      firstMouse_(true),
      mouseLookEnabled_(true),
      spaceWasPressed_(false)
{
    front_ = glm::normalize(front_);
}

void Camera::processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceWasPressed_)
    {
        mouseLookEnabled_ = !mouseLookEnabled_;
        spaceWasPressed_ = true;
        firstMouse_ = true;

        glfwSetInputMode(
            window,
            GLFW_CURSOR,
            mouseLookEnabled_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
        );
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        spaceWasPressed_ = false;
    }

    const float cameraSpeed = 4.0f * deltaTime;
    const glm::vec3 horizontalFront = glm::normalize(glm::vec3(front_.x, 0.0f, front_.z));
    const glm::vec3 horizontalRight = glm::normalize(glm::cross(horizontalFront, up_));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position_ += cameraSpeed * horizontalFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position_ -= cameraSpeed * horizontalFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position_ -= cameraSpeed * horizontalRight;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position_ += cameraSpeed * horizontalRight;

    position_.y = CAMERA_FIXED_HEIGHT;
}

void Camera::processMouseMovement(double xpos, double ypos)
{
    if (!mouseLookEnabled_)
    {
        return;
    }

    if (firstMouse_)
    {
        lastMouseX_ = static_cast<float>(xpos);
        lastMouseY_ = static_cast<float>(ypos);
        firstMouse_ = false;
    }

    float xoffset = static_cast<float>(xpos) - lastMouseX_;
    float yoffset = lastMouseY_ - static_cast<float>(ypos);

    lastMouseX_ = static_cast<float>(xpos);
    lastMouseY_ = static_cast<float>(ypos);

    constexpr float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (pitch_ > 89.0f) pitch_ = 89.0f;
    if (pitch_ < -89.0f) pitch_ = -89.0f;

    updateFront();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position_, position_ + front_, up_);
}

const glm::vec3& Camera::position() const
{
    return position_;
}

const glm::vec3& Camera::front() const
{
    return front_;
}

const glm::vec3& Camera::up() const
{
    return up_;
}

float Camera::zoom() const
{
    return fov_;
}

void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->processMouseMovement(xpos, ypos);
    }
}

void Camera::updateFront()
{
    glm::vec3 direction;
    direction.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    direction.y = std::sin(glm::radians(pitch_));
    direction.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front_ = glm::normalize(direction);
}
