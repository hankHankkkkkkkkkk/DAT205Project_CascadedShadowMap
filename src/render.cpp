#include "render.h"

#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
void DrawPlane(Shader& shader, const Mesh& plane)
{
    // Shared plane draw helper: each scene chooses the plane mesh that matches its scale.
    glm::mat4 planeModel = glm::mat4(1.0f);
    shader.setMat4("model", glm::value_ptr(planeModel));
    shader.setVec3("objectColor", 0.7f, 0.7f, 0.7f);

    glBindVertexArray(plane.vao);
    glDrawArrays(GL_TRIANGLES, 0, plane.vertexCount);
}

void DrawCube(
    Shader& shader,
    const Mesh& cube,
    const glm::vec3& position,
    const glm::vec3& scale,
    const glm::vec3& color)
{
    // Shared cube draw helper: the demo keeps using one cube mesh with per-object transforms.
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    shader.setMat4("model", glm::value_ptr(model));
    shader.setVec3("objectColor", color.x, color.y, color.z);

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
}

void RenderScene(
    Shader& shader,
    const Mesh& smallPlane,
    const Mesh& largePlane,
    const Mesh& cube,
    SceneMode sceneMode)
{
    // Scene dispatcher: both depth and color passes use the same scene selection.
    switch (sceneMode)
    {
    case SceneMode::Original:
        RenderOriginalScene(shader, smallPlane, cube);
        break;

    case SceneMode::CsmDemo:
    default:
        RenderCsmDemoScene(shader, largePlane, cube);
        break;
    }
}
