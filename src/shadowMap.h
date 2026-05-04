#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

struct ShadowMap
{
    unsigned int framebuffer = 0;
    unsigned int textureArray = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    int layers = 0;

    void destroy();
};

ShadowMap CreateShadowMap(unsigned int width, unsigned int height, int layers);

#endif
