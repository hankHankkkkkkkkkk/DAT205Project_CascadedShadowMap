#include "csm.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
std::vector<glm::vec4> GetSceneBoundsCorners(const glm::vec3& sceneMin, const glm::vec3& sceneMax)
{
    std::vector<glm::vec4> corners;
    corners.reserve(8);

    for (int x = 0; x < 2; ++x)
    {
        for (int y = 0; y < 2; ++y)
        {
            for (int z = 0; z < 2; ++z)
            {
                corners.emplace_back(
                    x == 0 ? sceneMin.x : sceneMax.x,
                    y == 0 ? sceneMin.y : sceneMax.y,
                    z == 0 ? sceneMin.z : sceneMax.z,
                    1.0f
                );
            }
        }
    }

    return corners;
}
}

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
    float padding,
    const glm::vec3& sceneMin,
    const glm::vec3& sceneMax,
    unsigned int shadowMapResolution,
    bool fitEntireScene,
    float* outLightDepthRange,
    float* outWorldTexelSize
)
{
    const glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    const glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

    const std::vector<glm::vec4> frustumCorners = GetFrustumCornersWorldSpace(projection, view);
    const std::vector<glm::vec4> sceneCorners = GetSceneBoundsCorners(sceneMin, sceneMax);
    // Single shadow maps use scene bounds so the one map covers the full demo area by default.
    const std::vector<glm::vec4>& corners = fitEntireScene ? sceneCorners : frustumCorners;

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

    float sceneMinZ = std::numeric_limits<float>::max();
    float sceneMaxZ = std::numeric_limits<float>::lowest();
    for (const glm::vec4& sceneCorner : sceneCorners)
    {
        const glm::vec4 trf = lightView * sceneCorner;
        sceneMinZ = std::min(sceneMinZ, trf.z);
        sceneMaxZ = std::max(sceneMaxZ, trf.z);
    }

    // The X/Y bounds stay fitted to the receiver slice, while Z covers scene casters
    // that can project into that slice along the directional light.
    const float orthoWidth = maxX - minX + padding * 2.0f;
    const float orthoHeight = maxY - minY + padding * 2.0f;
    const float worldTexelSize = std::max(orthoWidth, orthoHeight) / static_cast<float>(std::max(1u, shadowMapResolution));

    glm::vec2 snappedCenter(
        (minX + maxX) * 0.5f,
        (minY + maxY) * 0.5f
    );

    // Snap the light projection to shadow texels to avoid shimmering while the camera moves.
    if (worldTexelSize > 0.0f)
    {
        snappedCenter.x = std::floor(snappedCenter.x / worldTexelSize) * worldTexelSize;
        snappedCenter.y = std::floor(snappedCenter.y / worldTexelSize) * worldTexelSize;
    }

    const float lightMinX = snappedCenter.x - orthoWidth * 0.5f;
    const float lightMaxX = snappedCenter.x + orthoWidth * 0.5f;
    const float lightMinY = snappedCenter.y - orthoHeight * 0.5f;
    const float lightMaxY = snappedCenter.y + orthoHeight * 0.5f;

    const float casterMinZ = std::min(minZ, sceneMinZ);
    const float casterMaxZ = std::max(maxZ, sceneMaxZ);
    const float lightNearPlane = std::max(0.01f, -casterMaxZ - padding);
    const float lightFarPlane = std::max(lightNearPlane + 0.01f, -casterMinZ + padding);

    if (outLightDepthRange != nullptr)
    {
        *outLightDepthRange = lightFarPlane - lightNearPlane;
    }

    if (outWorldTexelSize != nullptr)
    {
        *outWorldTexelSize = worldTexelSize;
    }

    return glm::ortho(
        lightMinX,
        lightMaxX,
        lightMinY,
        lightMaxY,
        lightNearPlane,
        lightFarPlane
    ) * lightView;
}
