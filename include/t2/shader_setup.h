
#ifndef T2_SHADER_SETUP_H
#define T2_SHADER_SETUP_H

#include <OpenGL/gl.h>

typedef struct {
    GLuint shader_program;
    GLuint vertex_buffer;
    GLuint element_buffer;
} resources;

GLuint shader_setup(resources *res);

#endif
