
#ifndef T2_TEXT_H
#define T2_TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <OpenGL/gl.h>

#include <t2/config.h>

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
    int pixel_height;
};

struct text_configuration {
    int width;
    int height;
    GLuint shader_program;
    GLuint vao, vbo;
};

void logTextSystemInfo();
struct text_configuration* initializeText(struct configuration *main_config);
int loadFont(const char *font_filename, struct font *f, int pixel_height);
void renderText(struct text_configuration *config, struct font *font,
        const char *text, int len, GLfloat x, GLfloat y, GLfloat scale, float *color);

#endif
