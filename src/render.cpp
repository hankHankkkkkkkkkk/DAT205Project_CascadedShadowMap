#include "render.h"

#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
void DrawPlane(Shader& shader, const Mesh& plane, const glm::vec3& color = glm::vec3(0.7f))
{
    // Shared plane draw helper: each scene chooses the plane mesh that matches its scale.
    glm::mat4 planeModel = glm::mat4(1.0f);
    shader.setMat4("model", glm::value_ptr(planeModel));
    shader.setVec3("objectColor", color.x, color.y, color.z);
    shader.setFloat("objectAlpha", 1.0f);

    glBindVertexArray(plane.vao);
    glDrawArrays(GL_TRIANGLES, 0, plane.vertexCount);
}

void DrawCube(
    Shader& shader,
    const Mesh& cube,
    const glm::vec3& position,
    const glm::vec3& scale,
    const glm::vec3& color,
    float alpha = 1.0f)
{
    // Shared cube draw helper: the demo keeps using one cube mesh with per-object transforms.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    shader.setMat4("model", glm::value_ptr(model));
    shader.setVec3("objectColor", color.x, color.y, color.z);
    shader.setFloat("objectAlpha", alpha);

    glBindVertexArray(cube.vao);
    glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount);
}

void DrawRotatedCube(
    Shader& shader,
    const Mesh& cube,
    const glm::vec3& position,
    const glm::vec3& scale,
    const glm::vec3& rotationAxis,
    float rotationDegrees,
    const glm::vec3& color,
    float alpha = 1.0f)
{
    // Rotated thin cubes are used as simple glass slabs without adding a new mesh type.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationDegrees), rotationAxis);
    model = glm::scale(model, scale);

    shader.setMat4("model", glm::value_ptr(model));
    shader.setVec3("objectColor", color.x, color.y, color.z);
    shader.setFloat("objectAlpha", alpha);

    glBindVertexArray(cube.vao);
    glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount);
}

void RenderOriginalScene(Shader& shader, const Mesh& smallPlane, const Mesh& cube)
{
    // Original scene: preserve the first two-cube setup for direct comparison.
    DrawPlane(shader, smallPlane);

    DrawCube(
        shader,
        cube,
        glm::vec3(-1.0f, 0.5f, -1.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f, 0.5f, 0.2f)
    );

    DrawCube(
        shader,
        cube,
        glm::vec3(1.2f, 0.5f, 0.8f),
        glm::vec3(1.0f, 1.5f, 1.0f),
        glm::vec3(0.2f, 0.6f, 1.0f)
    );
}

void RenderCsmDemoScene(Shader& shader, const Mesh& largePlane, const Mesh& cube)
{
    // CSM demo scene: wide ground and distributed casters expose near/mid/far cascade behavior.
    DrawPlane(shader, largePlane);

    // Near field: small objects where shadow detail should be crisp.
    DrawCube(shader, cube, glm::vec3(-2.0f, 0.5f, -2.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.48f, 0.22f));
    DrawCube(shader, cube, glm::vec3(1.5f, 0.75f, -3.0f), glm::vec3(1.2f, 1.5f, 1.0f), glm::vec3(0.25f, 0.6f, 1.0f));
    DrawCube(shader, cube, glm::vec3(4.0f, 0.35f, -1.5f), glm::vec3(2.2f, 0.7f, 0.8f), glm::vec3(0.45f, 0.85f, 0.5f));

    // Mid field: taller casters make cascade transitions easier to inspect.
    DrawCube(shader, cube, glm::vec3(-5.0f, 2.5f, -10.0f), glm::vec3(0.8f, 5.0f, 0.8f), glm::vec3(0.78f, 0.72f, 0.62f));
    DrawCube(shader, cube, glm::vec3(2.5f, 1.5f, -14.0f), glm::vec3(6.0f, 3.0f, 0.45f), glm::vec3(0.58f, 0.66f, 0.76f));
    DrawCube(shader, cube, glm::vec3(8.0f, 3.0f, -18.0f), glm::vec3(1.0f, 6.0f, 1.0f), glm::vec3(0.72f, 0.55f, 0.42f));

    // Far field: long-distance casters expose why CSM matters.
    DrawCube(shader, cube, glm::vec3(-9.0f, 4.0f, -30.0f), glm::vec3(1.4f, 8.0f, 1.4f), glm::vec3(0.82f, 0.78f, 0.68f));
    DrawCube(shader, cube, glm::vec3(5.0f, 6.0f, -42.0f), glm::vec3(2.0f, 12.0f, 2.0f), glm::vec3(0.65f, 0.7f, 0.75f));
    DrawCube(shader, cube, glm::vec3(0.0f, 1.0f, -52.0f), glm::vec3(14.0f, 2.0f, 0.8f), glm::vec3(0.5f, 0.62f, 0.54f));
}

void RenderGlassOpaqueScene(Shader& shader, const Mesh& smallPlane, const Mesh& cube)
{
    // Glass scene keeps the original compact scale for shadow-technique comparisons.
    DrawPlane(shader, smallPlane, glm::vec3(1.0f));

    DrawCube(
        shader,
        cube,
        glm::vec3(1.65f, 0.55f, -1.2f),
        glm::vec3(0.8f, 1.0f, 0.8f),
        glm::vec3(0.2f, 0.2f, 0.2f)
    );
}

void RenderGlassPanes(Shader& shader, const Mesh& cube, float glassAlpha)
{
    // Two adjacent colored panes form one tilted transparent glass board.
    DrawRotatedCube(
        shader,
        cube,
        glm::vec3(-0.74f, 1.35f, -0.50f),
        glm::vec3(1.6f, 2.4f, 0.08f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -22.0f,
        glm::vec3(1.0f, 0.1f, 0.08f),
        glassAlpha
    );

    DrawRotatedCube(
        shader,
        cube,
        glm::vec3(0.74f, 1.35f, 0.10f),
        glm::vec3(1.6f, 2.4f, 0.08f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -22.0f,
        glm::vec3(0.08f, 0.25f, 1.0f),
        glassAlpha
    );
}

void RenderGlassScene(Shader& shader, const Mesh& smallPlane, const Mesh& cube, float glassAlpha)
{
    RenderGlassOpaqueScene(shader, smallPlane, cube);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    RenderGlassPanes(shader, cube, glassAlpha);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
}

void RenderScene(
    Shader& shader,
    const Mesh& smallPlane,
    const Mesh& largePlane,
    const Mesh& cube,
    SceneMode sceneMode,
    float glassAlpha)
{
    // Scene dispatcher: both depth and color passes use the same scene selection.
    switch (sceneMode)
    {
    case SceneMode::Original:
        RenderOriginalScene(shader, smallPlane, cube);
        break;

    case SceneMode::Glass:
        RenderGlassScene(shader, smallPlane, cube, glassAlpha);
        break;

    case SceneMode::CsmDemo:
    default:
        RenderCsmDemoScene(shader, largePlane, cube);
        break;
    }
}

void RenderSceneShadowCasters(
    Shader& shader,
    const Mesh& smallPlane,
    const Mesh& largePlane,
    const Mesh& cube,
    SceneMode sceneMode,
    bool includeGlassPanes)
{
    // Glass panes are treated as ordinary depth casters in the baseline shadow pass.
    switch (sceneMode)
    {
    case SceneMode::Glass:
        RenderGlassOpaqueScene(shader, smallPlane, cube);
        if (includeGlassPanes)
        {
            RenderGlassPanes(shader, cube, 1.0f);
        }
        break;

    default:
        RenderScene(shader, smallPlane, largePlane, cube, sceneMode);
        break;
    }
}

void RenderGlassStochasticCasters(Shader& shader, const Mesh& cube, float glassAlpha)
{
    // The stochastic pass draws only transparent panes into its colored shadow target.
    RenderGlassPanes(shader, cube, glassAlpha);
}
