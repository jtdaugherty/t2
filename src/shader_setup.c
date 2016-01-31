
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <t2/logging.h>
#include <t2/shader_setup.h>

#define MAX_SHADER_SIZE 0x100000

static void show_info_log(GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    log_error("%s", log);
    free(log);
}

GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source;
    GLuint shader;
    GLint shader_ok;

    struct stat st;
    FILE *fp;

    if (stat(filename, &st)) {
        log_error("Could not get file size for %s", filename);
        exit(1);
    }

    if (st.st_size > MAX_SHADER_SIZE) {
        log_error("File size %lld exceeds allowed size %d for file %s", st.st_size, MAX_SHADER_SIZE, filename);
        exit(1);
    }

    /* Load the source code containing the kernel*/
    fp = fopen(filename, "r");
    if (!fp) {
        log_error("Failed to open shader file %s.", filename);
        exit(1);
    }

    source = (char*)malloc(st.st_size);
    length = fread(source, 1, st.st_size, fp);
    fclose(fp);

    if (!source)
        exit(1);

    shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, &length);
    free(source);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        log_error("Failed to compile %s:", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        exit(1);
    }

    return shader;
}

GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        log_error("Failed to link shader program:");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

static GLuint make_buffer(GLenum target, const void *buffer_data, GLsizei buffer_size)
{
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(target, buffer);
    glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

GLuint shader_setup(glResources *res)
{
    GLuint vertex_shader, fragment_shader;

    // Quad vertex data
    static const GLfloat g_vertex_buffer_data[] = { 
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f
    };
    // Quad vertex ordering
    static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };
    
    res->vertex_buffer = make_buffer(GL_ARRAY_BUFFER, g_vertex_buffer_data, sizeof(g_vertex_buffer_data));
    res->element_buffer = make_buffer(GL_ELEMENT_ARRAY_BUFFER, g_element_buffer_data, sizeof(g_element_buffer_data));

    vertex_shader = make_shader(GL_VERTEX_SHADER, "shaders/t2.v.glsl");
    if (!vertex_shader) {
        log_error("Could not load vertex shader");
        return 1;
    }

    fragment_shader = make_shader(GL_FRAGMENT_SHADER, "shaders/t2.f.glsl");
    if (!fragment_shader) {
        log_error("Could not load fragment shader");
        return 1;
    }

    res->shader_program = make_program(vertex_shader, fragment_shader);
    if (!res->shader_program) {
        log_error("Could not load construct shader program");
        return 1;
    }

    res->position_attribute = glGetAttribLocation(res->shader_program, "position");
    if (res->position_attribute == -1) {
        log_error("Could not get position attribute location");
        exit(1);
    }

    res->texture_uniform = glGetUniformLocation(res->shader_program, "texture");
    if (res->texture_uniform == -1) {
        log_error("Could not get texture uniform location");
        exit(1);
    }

    return 0;
}
