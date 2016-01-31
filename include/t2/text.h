
#ifndef T2_TEXT_H
#define T2_TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <OpenGL/gl.h>

struct character {
    GLuint texture;
    GLuint width;
    GLuint rows;
    GLuint bitmap_left;
    GLuint bitmap_top;
    GLuint advance;
    int loaded;
};

#define FONT_NUM_CHARACTERS 128

struct font {
    struct character characters[FONT_NUM_CHARACTERS];
};

int loadFont(struct font *f);
void renderText(GLuint shader_program, int width, int height, struct font *font,
        const char *text, GLfloat x, GLfloat y, GLfloat scale, float *color);
void logTextSystemInfo();

#endif
