
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include <t2/version.h>
#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
#include <t2/util.h>
#include <t2/mathutil.h>
#include <t2/opencl_setup.h>
#include <t2/shader_setup.h>
#include <t2/texture.h>
#include <t2/samplers.h>

int global_log_level = LOG_DEBUG;

struct configuration {
    cl_uint traceDepth;
    int sampleRoot;
    int width;
    int height;
};

struct state {
    // Position vector
    cl_float position[3];

    // Heading vector
    cl_float heading[3];

    // Camera lens radius
    cl_float lens_radius;

    // Current sample index being rendered
    cl_uint sampleIdx;
};

/* Initial renderer state */
struct state programState = {
    .position = { 0, 1.0, -5.0 },
    .heading = { 0.0, 0.0, 1.0 },
    .lens_radius = 0.07,
    .sampleIdx = 0
};

/* Default configuration */
struct configuration config = {
    .traceDepth = 5,
    .sampleRoot = 1,
    .width = 640,
    .height = 480
};

/* Last known mouse cursor position for computing deltas during mouse
movement */
static double cursorX;
static double cursorY;

static inline void rotateHeading(cl_float angle)
{
    programState.heading[0] = cos(angle) * programState.heading[0] - sin(angle) * programState.heading[2];
    programState.heading[2] = sin(angle) * programState.heading[0] + cos(angle) * programState.heading[2];

    normalize(programState.heading);
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

        programState.sampleIdx = 0;
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

    if (DECREASE_DEPTH && config.traceDepth > 0) {
        programState.sampleIdx = 0;
        config.traceDepth = config.traceDepth == 0 ? 0 : config.traceDepth - 1;
        log_info("Trace depth: %d", config.traceDepth);
    }

    if (INCREASE_DEPTH) {
        programState.sampleIdx = 0;
        config.traceDepth++;
        log_info("Trace depth: %d", config.traceDepth);
    }

    if (DECREASE_RADIUS && programState.lens_radius > 0.0) {
        programState.lens_radius = MAXF(programState.lens_radius - 0.01, 0.0);
        programState.sampleIdx = 0;
        log_info("Lens radius: %f", programState.lens_radius);
    }

    if (INCREASE_RADIUS) {
        programState.lens_radius += 0.01;
        programState.sampleIdx = 0;
        log_info("Lens radius: %f", programState.lens_radius);
    }

    // Movement keys translate the position vector based on the heading
    float vel = 0.2;
    float headingX = 0.0;
    float headingZ = 0.0;

    if (MOVE_FORWARD) {
        headingX = vel * programState.heading[0];
        headingZ = vel * programState.heading[2];
    }

    if (MOVE_LEFT) {
        headingX = -1 * vel * programState.heading[0];
        headingZ = -1 * vel * programState.heading[2];
    }

    if (MOVE_RIGHT) {
        headingX = -1 * vel * programState.heading[2];
        headingZ = vel * programState.heading[0];
    }

    if (MOVE_BACKWARD) {
        headingX = vel * programState.heading[2];
        headingZ = -1 * vel * programState.heading[0];
    }

    if (headingX != 0 || headingZ != 0) {
        programState.sampleIdx = 0;
        programState.position[0] += headingX;
        programState.position[2] += headingZ;
    }
}

void logVersionInfo()
{
    int maj, min, rev;

    log_info("t2 version %s (commit %s)", T2_VERSION, T2_COMMIT);

    glfwGetVersion(&maj, &min, &rev);
    log_info("GLFW version: %d.%d.%d", maj, min, rev);

    log_info("GLEW version: %s", glewGetString(GLEW_VERSION));
}

int main()
{
    cl_platform_id platform_id = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_int ret = -1;
    int width, height;
    glResources res;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(config.width, config.height, "t2", NULL, NULL);
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

    logVersionInfo();

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwGetWindowSize(window, &width, &height);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* OpenCL initialization */

    /* Choose an OpenCL platform and create a context */
    platform_id = choosePlatform();
    logPlatformInfo(platform_id);
    cl_context context = createOpenCLContext();

    /* Choose an OpenCL device */
    cl_device_id device_id = chooseOpenCLDevice(platform_id, context);
    logDeviceInfo(device_id);

    /* Create a command queue for the device */
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret) {
        log_error("Could not create command queue, ret %d", ret);
        exit(1);
    }

    /* Create kernel program from the source */
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

    /* Set up GLSL shaders */
    ret = shader_setup(&res);

    /* Create rendering texture buffers */
    res.readTexture = make_texture(width, height);
    cl_mem texmemRead = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, res.readTexture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 1, ret %d", ret);
        exit(1);
    }

    res.writeTexture = make_texture(width, height);
    cl_mem texmemWrite = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, res.writeTexture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 2, ret %d", ret);
        exit(1);
    }

    int numSampleSets = width * 10;
    int samplesSize = sizeof(cl_float) * config.sampleRoot * config.sampleRoot * 2 * numSampleSets;

    log_info("Generating %d samples per pixel", config.sampleRoot * config.sampleRoot);
    log_info("Sample data:");
    log_info("  Types: square, disk");
    log_info("  %d sample sets per type", numSampleSets);
    log_info("  %d bytes memory allocated per type", samplesSize);

    /* Allocate and populate square sample sets */
    cl_float *squareSamples = malloc(samplesSize);
    for (int i = 0; i < numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * config.sampleRoot * config.sampleRoot);
        generateJitteredSampleSet(squareSamples + offset, config.sampleRoot, NULL);
    }

    /* Set up OpenCL buffer reference to square sample memory */
    cl_mem squareSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, squareSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        exit(1);
    }

    /* Allocate and populate disk sample sets */
    cl_float *diskSamples = malloc(samplesSize);
    for (int i = 0; i < numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * config.sampleRoot * config.sampleRoot);
        generateJitteredSampleSet(diskSamples + offset, config.sampleRoot, mapToUnitDisk);
    }

    /* Set up OpenCL buffer reference to disk sample memory */
    cl_mem diskSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, diskSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        exit(1);
    }

    char title[64];
    cl_uint maxSamples = config.sampleRoot * config.sampleRoot;

    glfwSwapInterval(1);
    glGenFramebuffers(1, &res.fbo);

    log_info("Ready.");

    while (!glfwWindowShouldClose(window))
    {
        if (programState.sampleIdx < maxSamples) {
            /* Before we begin collecting a new sample, copy the old
               write buffer to the read buffer. This is critical because
               the kernel reads the image data and averages new sample
               data with it before writing back out, so we need to read
               this time what we wrote last time. Since OpenCL doesn't
               support read-write images, we have to have two: one to
               read, one to write, and some code to copy between them at
               the right time (now). */
            copyTexture(res.fbo, res.writeTexture, res.readTexture, width, height);

            /* Set OpenCL Kernel Parameters */
            ret = 0;
            ret |= clSetKernelArg(kernel, 0,    sizeof(cl_mem),      &texmemRead);
            ret |= clSetKernelArg(kernel, 1,    sizeof(cl_mem),      &texmemWrite);
            ret |= clSetKernelArg(kernel, 2,    sizeof(width),       &width);
            ret |= clSetKernelArg(kernel, 3,    sizeof(height),      &height);
            ret |= clSetKernelArg(kernel, 4,    sizeof(programState.position),    programState.position);
            ret |= clSetKernelArg(kernel, 5,    sizeof(programState.heading),     programState.heading);
            ret |= clSetKernelArg(kernel, 6,    sizeof(programState.lens_radius), &programState.lens_radius);
            ret |= clSetKernelArg(kernel, 7,    sizeof(cl_mem),      &squareSampleBuf);
            ret |= clSetKernelArg(kernel, 8,    sizeof(cl_mem),      &diskSampleBuf);
            ret |= clSetKernelArg(kernel, 9,    sizeof(cl_int),      &numSampleSets);
            ret |= clSetKernelArg(kernel, 10,   sizeof(cl_int),      &config.sampleRoot);
            ret |= clSetKernelArg(kernel, 11,   sizeof(programState.sampleIdx),   &programState.sampleIdx);
            ret |= clSetKernelArg(kernel, 12,   sizeof(config.traceDepth),  &config.traceDepth);

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

            programState.sampleIdx++;

            snprintf(title, sizeof(title), "t2 [%d/%d samples]", programState.sampleIdx, maxSamples);
            glfwSetWindowTitle(window, title);

            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(res.shader_program);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, res.writeTexture);
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
