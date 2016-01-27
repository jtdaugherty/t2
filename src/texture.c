
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
