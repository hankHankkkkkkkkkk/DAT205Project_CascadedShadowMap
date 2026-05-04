#ifndef RENDER_H
#define RENDER_H

#include "mesh.h"

class Shader;

void RenderScene(Shader& shader, const Mesh& plane, const Mesh& cube);

#endif
