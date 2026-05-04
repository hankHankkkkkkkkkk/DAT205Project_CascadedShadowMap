#include "mesh.h"

#include <glad/glad.h>

namespace
{
Mesh CreateMesh(const float* vertices, unsigned int vertexBytes, int vertexCount)
{
    Mesh mesh;
    mesh.vertexCount = vertexCount;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return mesh;
}
}

void Mesh::destroy()
{
    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    vertexCount = 0;
}

Mesh CreateCubeMesh()
{
    const float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f
    };

    return CreateMesh(cubeVertices, sizeof(cubeVertices), 36);
}

Mesh CreateSmallPlaneMesh()
{
    // Original scene ground plane: this preserves the small test scene scale.
    const float planeVertices[] = {
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f,
         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,

         5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        -5.0f, 0.0f,  5.0f,   0.0f, 1.0f, 0.0f,
        -5.0f, 0.0f, -5.0f,   0.0f, 1.0f, 0.0f
    };

    return CreateMesh(planeVertices, sizeof(planeVertices), 6);
}

Mesh CreateLargePlaneMesh()
{
    // CSM demo ground plane: the larger receiver makes long-range cascades visible.
    const float planeVertices[] = {
        -60.0f, 0.0f, -60.0f,   0.0f, 1.0f, 0.0f,
         60.0f, 0.0f, -60.0f,   0.0f, 1.0f, 0.0f,
         60.0f, 0.0f,  60.0f,   0.0f, 1.0f, 0.0f,

         60.0f, 0.0f,  60.0f,   0.0f, 1.0f, 0.0f,
        -60.0f, 0.0f,  60.0f,   0.0f, 1.0f, 0.0f,
        -60.0f, 0.0f, -60.0f,   0.0f, 1.0f, 0.0f
    };

    return CreateMesh(planeVertices, sizeof(planeVertices), 6);
}
