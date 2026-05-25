#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

struct ShadowMap
{
    unsigned int framebuffer = 0;
    unsigned int textureArray = 0;
    unsigned int stochasticFramebuffer = 0;
    unsigned int stochasticColorTexture = 0;
    unsigned int stochasticDepthTexture = 0;
    unsigned int deepFramebuffer = 0;
    unsigned int deepColorTextureArray = 0;
    unsigned int deepDepthTextureArray = 0;
    unsigned int deepScratchDepthTexture = 0;
    unsigned int width = 0;
    unsigned int height = 0;
    int layers = 0;

    void destroy();
};

ShadowMap CreateShadowMap(unsigned int width, unsigned int height, int layers);

#endif
