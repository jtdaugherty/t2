
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <string.h>

#include <t2/texture.h>

GLuint make_texture(int width, int height)
{
    GLuint texture;

    unsigned char *buf = malloc(width * height * sizeof(float) * 4);
    memset(buf, 0, width * height * sizeof(float) * 4);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_RGBA,
        width, height, 0,
        GL_RGBA, GL_FLOAT,
        buf
    );

    free(buf);

    return texture;
}

/**
 * This is only to be used during the rendering loop on an FBO and
 * textures that have already been set up.
 */
void copyTexture(GLuint fbo, GLuint texSrc, GLuint texDst,
        int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, texSrc, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
            GL_TEXTURE_2D, texDst, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
