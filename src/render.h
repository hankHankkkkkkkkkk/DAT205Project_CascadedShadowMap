#ifndef RENDER_H
#define RENDER_H

#include "gui.h"
#include "mesh.h"

class Shader;

void RenderScene(
    Shader& shader,
    const Mesh& smallPlane,
    const Mesh& largePlane,
    const Mesh& cube,
    SceneMode sceneMode
);

#endif
