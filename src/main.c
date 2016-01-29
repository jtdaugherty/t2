
#include <GL/glew.h>

#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include <t2/version.h>
#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
#include <t2/util.h>
#include <t2/opencl_setup.h>
#include <t2/shader_setup.h>
#include <t2/texture.h>

#define MAXF(a, b) ((a) > (b) ? (a) : (b))
 
int global_log_level = LOG_DEBUG;

// Position vector
cl_float position[3] = { 0, 1.0, -5.0 };

// Heading vector
cl_float heading[3] = { 0.0, 0.0, 1.0 };

// Camera lens radius
cl_float lens_radius = 0.07;

cl_uint sampleIdx = 0;

cl_uint traceDepth = 5;

double cursorX;
double cursorY;

float randFloat()
{
    return ((float)(arc4random() % ((unsigned)RAND_MAX + 1))) /
        ((float)((unsigned)RAND_MAX + 1));
}

void normalize(cl_float *vec)
{
    cl_float len = sqrt(vec[0] * vec[0] +
            vec[1] * vec[1] +
            vec[2] * vec[2]);

    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

void rotateHeading(cl_float angle)
{
    heading[0] = cos(angle) * heading[0] - sin(angle) * heading[2];
    heading[2] = sin(angle) * heading[0] + cos(angle) * heading[2];

    normalize(heading);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &cursorX, &cursorY);
    } else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
    {
        float angleDiff = (float) (x - cursorX) / 80.f;

        // Rotate the heading vector by this much
        rotateHeading(angleDiff);

        cursorX = x;
        cursorY = y;

        sampleIdx = 0;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
#define PRESS(KEY) (key == KEY && (action == GLFW_PRESS || action == GLFW_REPEAT))
#define SHIFT (mods & GLFW_MOD_SHIFT)

#define QUIT            (PRESS(GLFW_KEY_ESCAPE) || PRESS(GLFW_KEY_Q))
#define MOVE_BACKWARD   (PRESS(GLFW_KEY_A))
#define MOVE_RIGHT      (PRESS(GLFW_KEY_D))
#define MOVE_FORWARD    (PRESS(GLFW_KEY_W))
#define MOVE_LEFT       (PRESS(GLFW_KEY_S))
#define DECREASE_DEPTH  (PRESS(GLFW_KEY_MINUS))
#define INCREASE_DEPTH  (PRESS(GLFW_KEY_EQUAL) && SHIFT)
#define DECREASE_RADIUS (PRESS(GLFW_KEY_R) && (!SHIFT))
#define INCREASE_RADIUS (PRESS(GLFW_KEY_R) && SHIFT)

    if (QUIT)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (DECREASE_DEPTH && traceDepth > 0) {
        sampleIdx = 0;
        traceDepth = traceDepth == 0 ? 0 : traceDepth - 1;
        log_info("Trace depth: %d", traceDepth);
    }

    if (INCREASE_DEPTH) {
        sampleIdx = 0;
        traceDepth++;
        log_info("Trace depth: %d", traceDepth);
    }

    if (DECREASE_RADIUS && lens_radius > 0.0) {
        lens_radius = MAXF(lens_radius - 0.01, 0.0);
        sampleIdx = 0;
        log_info("Lens radius: %f", lens_radius);
    }

    if (INCREASE_RADIUS) {
        lens_radius += 0.01;
        sampleIdx = 0;
        log_info("Lens radius: %f", lens_radius);
    }

    // Movement keys translate the position vector based on the heading
    float vel = 0.2;
    float headingX = 0.0;
    float headingZ = 0.0;

    if (MOVE_FORWARD) {
        headingX = vel * heading[0];
        headingZ = vel * heading[2];
    }

    if (MOVE_LEFT) {
        headingX = -1 * vel * heading[0];
        headingZ = -1 * vel * heading[2];
    }

    if (MOVE_RIGHT) {
        headingX = -1 * vel * heading[2];
        headingZ = vel * heading[0];
    }

    if (MOVE_BACKWARD) {
        headingX = vel * heading[2];
        headingZ = -1 * vel * heading[0];
    }

    if (headingX != 0 || headingZ != 0)
        sampleIdx = 0;

    position[0] += headingX;
    position[2] += headingZ;
}

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

void mapToUnitDisk(float *x, float *y)
{
    float spX, spY, phi, r;

    spX = 2.0 * (*x) - 1.0;
    spY = 2.0 * (*y) - 1.0;

    if (spX > -spY) {
        if (spX > spY) {
            r = spX;
            phi = spY / spX;
        } else {
            r = spY;
            phi = 2.0 - spX / spY;
        }
    } else {
        if (spX < spY) {
            r = -spX;
            phi = 4 + spY / spX;
        } else {
            r = -spY;
            if (spY != 0.0) {
                phi = 6.0 - spX / spY;
            } else {
                phi = 0.0;
            }
        }
    }

    phi *= M_PI / 4.0;
    *x = r * cos(phi);
    *y = r * sin(phi);
}

void generateRandomSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*))
{
    // float r2 = (float)(sampleRoot * sampleRoot);
    for (int i = 0; i < sampleRoot; i++) {
        for (int j = 0; j < sampleRoot; j++) {
            // float a = randFloat();
            // float b = randFloat();
            // float littleI = sampleRoot - 1 - i;
            // float littleJ = sampleRoot - 1 - j;

            // float x = ((float)i) / ((float)sampleRoot) + (littleI + a) / r2;
            // float y = ((float)j) / ((float)sampleRoot) + (littleJ + b) / r2;
            float x = randFloat();
            float y = randFloat();

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }
}

void generateJitteredSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*))
{
    float inc = 1.0 / ((float) sampleRoot);
    for (int i = 0; i < sampleRoot; i++) {
        for (int j = 0; j < sampleRoot; j++) {
            float x = (i * inc) + randFloat() * inc;
            float y = (j * inc) + randFloat() * inc;

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }
}

int main()
{
    cl_platform_id platform_id = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_int ret = -1;
    int width, height;
    resources res;

    log_info("t2 version %s (commit %s)", T2_VERSION, T2_COMMIT);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(1024, 768, "t2", NULL, NULL);
    if (!window)
    {
        log_error("Could not create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        log_error("GLEW initialization failed: %s", glewGetErrorString(err));
        exit(1);
    }

    int maj, min, rev;
    glfwGetVersion(&maj, &min, &rev);
    log_info("GLFW version: %d.%d.%d", maj, min, rev);

    log_info("GLEW version: %s", glewGetString(GLEW_VERSION));
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* OpenCL initialization */

    /* Choose an OpenCL platform */
    platform_id = choosePlatform();
    logPlatformInfo(platform_id);
    cl_context context = createOpenCLContext();

    cl_device_id device_id = chooseOpenCLDevice(platform_id, context);
    logDeviceInfo(device_id);

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret) {
        log_error("Could not create command queue, ret %d", ret);
        exit(1);
    }

    /* Create Kernel Program from the source */
    program = readAndBuildProgram(context, device_id, "cl/t2.cl", &ret);
    if (!program) {
        log_error("readAndBuildProgram failed, ret %d", ret);
        exit(1);
    }

    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "raytracer", &ret);
    if (ret) {
        log_error("Could not create kernel!\n");
        exit(1);
    }

    ret = shader_setup(&res);

    glfwGetWindowSize(window, &width, &height);
    glfwSwapInterval(1);

    GLuint textureRead = make_texture(width, height);
    cl_mem texmemRead = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, textureRead, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 1, ret %d", ret);
        exit(1);
    }

    GLuint textureWrite = make_texture(width, height);
    cl_mem texmemWrite = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, textureWrite, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 2, ret %d", ret);
        exit(1);
    }

    int sampleRoot = 10;

    log_info("Generating %d samples per pixel", sampleRoot * sampleRoot);

    int numSampleSets = width * 10;
    int samplesSize = sizeof(cl_float) * sampleRoot * sampleRoot * 2 * numSampleSets;

    log_info("Samples:");
    log_info("  Types: square, disk");
    log_info("  %d sample sets per type", numSampleSets);
    log_info("  %d samples per set", sampleRoot * sampleRoot);
    log_info("  %d bytes memory allocated per type", samplesSize);

    cl_float *squareSamples = malloc(samplesSize);

    for (int i = 0; i < numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * sampleRoot * sampleRoot);
        generateJitteredSampleSet(squareSamples + offset, sampleRoot, NULL);
    }

    cl_mem squareSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, squareSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        exit(1);
    }

    cl_float *diskSamples = malloc(samplesSize);

    for (int i = 0; i < numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * sampleRoot * sampleRoot);
        generateJitteredSampleSet(diskSamples + offset, sampleRoot, mapToUnitDisk);
    }

    cl_mem diskSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, diskSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        exit(1);
    }

    cl_uint maxSamples = sampleRoot * sampleRoot;

    GLuint fbo;
    glGenFramebuffers(1, &fbo);

    char title[64];

    log_info("Ready.");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        if (sampleIdx < maxSamples) {
            copyTexture(fbo, textureWrite, textureRead, width, height);

            /* Set OpenCL Kernel Parameters */
            ret = 0;
            ret |= clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&texmemRead);
            ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&texmemWrite);
            ret |= clSetKernelArg(kernel, 2, sizeof(width), (void *)&width);
            ret |= clSetKernelArg(kernel, 3, sizeof(height), (void *)&height);
            ret |= clSetKernelArg(kernel, 4, sizeof(position), (void *)position);
            ret |= clSetKernelArg(kernel, 5, sizeof(heading), (void *)heading);
            ret |= clSetKernelArg(kernel, 6, sizeof(lens_radius), (void *)&lens_radius);
            ret |= clSetKernelArg(kernel, 7, sizeof(cl_mem), (void *)&squareSampleBuf);
            ret |= clSetKernelArg(kernel, 8, sizeof(cl_mem), (void *)&diskSampleBuf);
            ret |= clSetKernelArg(kernel, 9, sizeof(cl_int), (void *)&numSampleSets);
            ret |= clSetKernelArg(kernel, 10, sizeof(cl_int), (void *)&sampleRoot);
            ret |= clSetKernelArg(kernel, 11, sizeof(sampleIdx), (void *)&sampleIdx);
            ret |= clSetKernelArg(kernel, 12, sizeof(traceDepth), (void *)&traceDepth);

            if (ret) {
                log_error("Could not set kernel argument, ret %d", ret);
                exit(1);
            }

            ret = clEnqueueAcquireGLObjects(command_queue, 1, &texmemRead, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue GL object acquisition, ret %d", ret);
                exit(1);
            }

            ret = clEnqueueAcquireGLObjects(command_queue, 1, &texmemWrite, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue GL object acquisition, ret %d", ret);
                exit(1);
            }

            /* Execute OpenCL Kernel */
            size_t global_work_size[2] = { width, height };
            ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue task");
                exit(1);
            }

            // Before returning the objects to OpenGL, we sync to make sure OpenCL is done.
            ret = clEnqueueReleaseGLObjects(command_queue, 1, &texmemRead, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue GL object acquisition, ret %d", ret);
                exit(1);
            }

            ret = clEnqueueReleaseGLObjects(command_queue, 1, &texmemWrite, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue GL object acquisition, ret %d", ret);
                exit(1);
            }

            sampleIdx++;

            snprintf(title, sizeof(title), "t2 [%d/%d samples]", sampleIdx, maxSamples);
            glfwSetWindowTitle(window, title);

            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(res.shader_program);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureWrite);
            glUniform1i(res.texture_uniform, 0);

            glBindBuffer(GL_ARRAY_BUFFER, res.vertex_buffer);
            glVertexAttribPointer(
                    res.position_attribute,          /* attribute */
                    2,                                /* size */
                    GL_FLOAT,                         /* type */
                    GL_FALSE,                         /* normalized? */
                    sizeof(GLfloat)*2,                /* stride */
                    (void*)0                          /* array buffer offset */
                    );
            glEnableVertexAttribArray(res.position_attribute);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.element_buffer);
            glDrawElements(
                    GL_TRIANGLE_STRIP,  /* mode */
                    4,                  /* count */
                    GL_UNSIGNED_SHORT,  /* type */
                    (void*)0            /* element array buffer offset */
                    );

            glDisableVertexAttribArray(res.position_attribute);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);
        }

        /* Poll for and process events */
        glfwPollEvents();
    }

    /* Finalization */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
