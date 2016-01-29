
#ifndef T2_TEXTURE_H
#define T2_TEXTURE_H

#include <OpenGL/gl.h>

GLuint make_texture(int width, int height);
void copyTexture(GLuint fbo, GLuint texSrc, GLuint texDst, int width, int height);

#endif
