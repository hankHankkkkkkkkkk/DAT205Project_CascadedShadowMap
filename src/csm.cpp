#include "csm.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

void UpdateCascadeSplits(float* cascadeSplits, int cascadeCount, float nearPlane, float farPlane, float lambda)
{
    for (int i = 0; i < cascadeCount; ++i)
    {
        const float p = static_cast<float>(i + 1) / static_cast<float>(cascadeCount);
        const float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
        const float linearSplit = nearPlane + (farPlane - nearPlane) * p;
        cascadeSplits[i] = lambda * logSplit + (1.0f - lambda) * linearSplit;
    }
}

std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view)
{
    const glm::mat4 inverse = glm::inverse(projection * view);
    std::vector<glm::vec4> corners;
    corners.reserve(8);

    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z)
            {
                const glm::vec4 point = inverse * glm::vec4(
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

glm::mat4 GetLightSpaceMatrix(
    float nearPlane,
    float farPlane,
    float aspect,
    float fov,
    const glm::vec3& cameraPosition,
    const glm::vec3& cameraFront,
    const glm::vec3& cameraUp,
    const glm::vec3& lightDirection,
    float padding
)
{
    const glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

    const std::vector<glm::vec4> corners = GetFrustumCornersWorldSpace(projection, view);

    glm::vec3 center(0.0f);
    for (const glm::vec4& corner : corners)
    {
        center += glm::vec3(corner);
    }
    center /= static_cast<float>(corners.size());

    const glm::vec3 lightDir = glm::normalize(lightDirection);
    const glm::mat4 lightView = glm::lookAt(
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
        const glm::vec4 trf = lightView * corner;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    const float lightNearPlane = std::max(0.01f, -maxZ - padding);
    const float lightFarPlane = std::max(lightNearPlane + 0.01f, -minZ + padding);

    return glm::ortho(
        minX - padding,
        maxX + padding,
        minY - padding,
        maxY + padding,
        lightNearPlane,
        lightFarPlane
    ) * lightView;
}
