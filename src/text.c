
#include <t2/logging.h>
#include <t2/text.h>

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_FILENAME        "fonts/InputMono-Regular.ttf"

static int ft_initialized = 0;
static FT_Library ft;

static int ensureFreetypeInitialized()
{
    if (ft_initialized)
        return 0;

    if (FT_Init_FreeType(&ft)) {
        log_error("Could not initialize FreeType library");
        return 1;
    }

    ft_initialized = 1;
    return 0;
}

int loadFont(struct font *f)
{
    int ret;
    ensureFreetypeInitialized();

    FT_Face face;
    if (FT_New_Face(ft, FONT_FILENAME, 0, &face)) {
        log_error("Could not open font %s", FONT_FILENAME);
        return 1;
    }

    // First, mark all characters as not loaded
    for (GLubyte i = 0; i < FONT_NUM_CHARACTERS; i++) {
        f->characters[i].loaded = 0;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Set_Pixel_Sizes(face, 0, 18);

    // Load each character that we care about
    for (GLubyte c = 0; c < FONT_NUM_CHARACTERS; c++) {
        ret = FT_Load_Char(face, c, FT_LOAD_RENDER);
        if (ret) {
            log_warn("Failed to load glyph at index %d, ret %d", c, ret);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Now store character for later use
        f->characters[c].texture = texture;
        f->characters[c].width = face->glyph->bitmap.width;
        f->characters[c].rows = face->glyph->bitmap.rows;
        f->characters[c].bitmap_left = face->glyph->bitmap_left;
        f->characters[c].bitmap_top = face->glyph->bitmap_top;
        f->characters[c].advance = face->glyph->advance.x;

        f->characters[c].loaded = 1;
    }

    FT_Done_Face(face);

    return 0;
}

void renderText(GLuint shader_program, int width, int height, struct font *font,
        const char *text, GLfloat x, GLfloat y, GLfloat scale, float *color)
{
    GLuint VAO, VBO;
    glGenVertexArraysAPPLE(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArrayAPPLE(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArrayAPPLE(0);

    glUseProgram(shader_program);

    // Activate corresponding render state	
    glUniform3f(glGetUniformLocation(shader_program, "textColor"),
            color[0], color[1], color[2]);
    glUniform1i(glGetUniformLocation(shader_program, "width"), width);
    glUniform1i(glGetUniformLocation(shader_program, "height"), height);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArrayAPPLE(VAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int len = strlen(text);
    for (int c = 0; c < len; c++) {
        if (c < 0 || c >= FONT_NUM_CHARACTERS)
            log_warn("Invalid character index %d", c);

        struct character ch = font->characters[(int) text[c]];
        if (!ch.loaded)
            log_error("Requested character '%c' is not loaded", text[c]);
        else {
            log_debug("Character loaded: '%c'", text[c]);
            log_debug("  width: %d", ch.width);
            log_debug("  rows: %d", ch.rows);
        }

        GLfloat xpos = x + ch.bitmap_left * scale;
        GLfloat ypos = y - (ch.rows - ch.bitmap_top) * scale;

        GLfloat w = ch.width * scale;
        GLfloat h = ch.rows * scale;

        log_debug("x %f y %f", x, y);
        log_debug("xpos, ypos %f %f", xpos, ypos);

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursor for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glDisable(GL_BLEND);

    glBindVertexArrayAPPLE(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void logTextSystemInfo()
{
    ensureFreetypeInitialized();

    FT_Int maj, min, patch;
    FT_Library_Version(ft, &maj, &min, &patch);
    log_info("FreeType version: %d.%d.%d", maj, min, patch);
}
