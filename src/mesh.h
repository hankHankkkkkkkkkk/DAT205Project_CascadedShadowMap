#ifndef MESH_H
#define MESH_H

struct Mesh
{
    unsigned int vao = 0;
    unsigned int vbo = 0;
    int vertexCount = 0;

    void destroy();
};

Mesh CreateCubeMesh();
Mesh CreatePlaneMesh();

#endif
