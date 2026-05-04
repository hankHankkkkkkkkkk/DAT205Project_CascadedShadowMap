#include "shadowMap.h"

#include <glad/glad.h>

void ShadowMap::destroy()
{
    if (framebuffer != 0)
    {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }

    if (textureArray != 0)
    {
        glDeleteTextures(1, &textureArray);
        textureArray = 0;
    }
}

ShadowMap CreateShadowMap(unsigned int width, unsigned int height, int layers)
{
    ShadowMap shadowMap;
    shadowMap.width = width;
    shadowMap.height = height;
    shadowMap.layers = layers;

    glGenFramebuffers(1, &shadowMap.framebuffer);

    glGenTextures(1, &shadowMap.textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap.textureArray);

    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT32F,
        width,
        height,
        layers,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.framebuffer);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return shadowMap;
}
