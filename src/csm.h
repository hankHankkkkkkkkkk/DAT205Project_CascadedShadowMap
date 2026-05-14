#ifndef CSM_H
#define CSM_H

#include <glm/glm.hpp>

#include <vector>

void UpdateCascadeSplits(float* cascadeSplits, int cascadeCount, float nearPlane, float farPlane, float lambda);

std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view);

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
);

#endif
