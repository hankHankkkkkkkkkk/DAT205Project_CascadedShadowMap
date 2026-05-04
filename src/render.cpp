#include "render.h"

#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void RenderScene(Shader& shader, const Mesh& plane, const Mesh& cube)
{
    glm::mat4 planeModel = glm::mat4(1.0f);
    shader.setMat4("model", glm::value_ptr(planeModel));
    shader.setVec3("objectColor", 0.7f, 0.7f, 0.7f);
    glBindVertexArray(plane.vao);
    glDrawArrays(GL_TRIANGLES, 0, plane.vertexCount);

    glm::mat4 cubeModel1 = glm::mat4(1.0f);
    cubeModel1 = glm::translate(cubeModel1, glm::vec3(-1.0f, 0.5f, -1.0f));
    shader.setMat4("model", glm::value_ptr(cubeModel1));
    shader.setVec3("objectColor", 1.0f, 0.5f, 0.2f);
    glBindVertexArray(cube.vao);
    glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount);

    glm::mat4 cubeModel2 = glm::mat4(1.0f);
    cubeModel2 = glm::translate(cubeModel2, glm::vec3(1.2f, 0.5f, 0.8f));
    cubeModel2 = glm::scale(cubeModel2, glm::vec3(1.0f, 1.5f, 1.0f));
    shader.setMat4("model", glm::value_ptr(cubeModel2));
    shader.setVec3("objectColor", 0.2f, 0.6f, 1.0f);
    glBindVertexArray(cube.vao);
    glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount);
}
