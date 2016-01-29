
#ifndef T2_SHADER_SETUP_H
#define T2_SHADER_SETUP_H

#include <OpenGL/gl.h>

typedef struct {
    GLuint shader_program;
    GLuint vertex_buffer;
    GLuint element_buffer;

    GLint position_attribute;
    GLint texture_uniform;

    GLuint readTexture;
    GLuint writeTexture;
    GLuint fbo;
} glResources;

GLuint shader_setup(glResources *res);

#endif
