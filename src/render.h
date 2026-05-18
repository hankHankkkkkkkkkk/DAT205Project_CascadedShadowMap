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
    SceneMode sceneMode,
    float glassAlpha = 0.45f
);

void RenderSceneShadowCasters(
    Shader& shader,
    const Mesh& smallPlane,
    const Mesh& largePlane,
    const Mesh& cube,
    SceneMode sceneMode,
    bool includeGlassPanes = true
);

void RenderGlassStochasticCasters(
    Shader& shader,
    const Mesh& cube,
    float glassAlpha
);

#endif
