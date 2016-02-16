
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <sys/time.h>

#include <t2/args.h>
#include <t2/config.h>
#include <t2/device.h>
#include <t2/info.h>
#include <t2/logging.h>
#include <t2/mathutil.h>
#include <t2/opencl_setup.h>
#include <t2/overlay.h>
#include <t2/platform.h>
#include <t2/samplers.h>
#include <t2/shader_setup.h>
#include <t2/state.h>
#include <t2/texture.h>
#include <t2/util.h>

/* Initial renderer state */
struct state programState = {
    .position = { { 0, 1.0, -5.0 } },
    .heading = { { 0.0, 0.0, 1.0 } },
    .lens_radius = 0,
    .sampleNum = 0,
    .show_overlay = 1,
    .last_frame_time = -1
};

/* Default configuration */
struct configuration config = {
    .traceDepth = 5,
    .sampleRoot = 1,
    .width = 800,
    .height = 600,
    .logLevel = LOG_INFO,
    .batchSize = 1
};

struct sample_data {
    cl_float *squareSamples;
    cl_mem squareSampleBuf;

    cl_float *diskSamples;
    cl_mem diskSampleBuf;

    size_t numSampleSets;
};

/* OpenCL buffers for configuration and state */
cl_mem configBuf = NULL;
cl_mem stateBuf = NULL;

/* Dirty flags */
int dirty_config = 1;
int dirty_state = 1;

/* OpenCL command queue. This is global because updateConfigBuffer
needs to be able to selectively issue an OpenCL buffer update when the
configuration changes. */
cl_command_queue command_queue = NULL;

/* Where sample buffer pointers are kept. These are global so that GLFW
handlers can trigger sample allocations and update this structure. */
struct sample_data samples = { NULL, NULL, NULL, NULL, 0 };

/* For logging.h to get access to the global log level */
int *global_log_level = &config.logLevel;

/* The OpenCL context. This is global so that we can access it in the
GLFW handlers since they don't support passing in a void * user pointer.
*/
cl_context context = NULL;

/* Store the old configured batch size here while a key or mouse button
is held down */
cl_uint oldBatchSize = -1;

/* Last known mouse cursor position for computing deltas during mouse
movement */
static double cursorX;
static double cursorY;

/* Button press mask and bits for tracking whether to reduce or restore
the rendering sample batch size */
uint8_t button_mask = 0;
#define KB_PRESSED      (1 << 0)
#define MOUSE_PRESSED   (1 << 1)
#define ANY_PRESSED     ((button_mask & (KB_PRESSED | MOUSE_PRESSED)) != 0)
#define NONE_PRESSED    ((button_mask & (KB_PRESSED | MOUSE_PRESSED)) == 0)

static inline void restartRendering()
{
    programState.sampleNum = 0;
}

static inline void updateConfigBuffer()
{
    if (dirty_config) {
        log_debug("Configuration changed, updating");
        dirty_config = 0;
        int ret = clEnqueueWriteBuffer(command_queue, configBuf, 1, 0, sizeof(struct configuration),
                &config, 0, NULL, NULL);
        if (ret) {
            log_error("Error updating configuration buffer, ret %d", ret);
            exit(1);
        }
    }
}

static inline void markConfigDirty()
{
    dirty_config = 1;
}

static inline void updateStateBuffer()
{
    if (dirty_state) {
        dirty_state = 0;
        int ret = clEnqueueWriteBuffer(command_queue, stateBuf, 1, 0, sizeof(struct state),
                &programState, 0, NULL, NULL);
        if (ret) {
            log_error("Error updating configuration buffer, ret %d", ret);
            exit(1);
        }
    }
}

static inline void markStateDirty()
{
    dirty_state = 1;
}

int setup_samples(struct sample_data *s, int sampleRoot, struct configuration *cfg, cl_context context)
{
    int ret;

    s->numSampleSets = cfg->width * 23.5;
    size_t samplesSize = sizeof(cl_float) * sampleRoot * sampleRoot * 2 *
        s->numSampleSets;

    if (s->squareSamples) {
        log_info("Freeing old square samples");
        free(s->squareSamples);
        clReleaseMemObject(s->squareSampleBuf);
    }

    if (s->diskSamples) {
        log_info("Freeing old disk samples");
        free(s->diskSamples);
        clReleaseMemObject(s->diskSampleBuf);
    }

    log_info("Generating %d samples per pixel", sampleRoot * sampleRoot);
    log_info("  Types: square, disk");
    log_info("  %ld sample sets per type", s->numSampleSets);
    log_info("  %ld bytes memory allocated per type", samplesSize);

    log_info("Generating square samples...");
    /* Allocate and populate square sample sets */
    s->squareSamples = malloc(samplesSize);
    if (!s->squareSamples) {
        log_error("Could not allocate %ld bytes of memory for square samples", samplesSize);
        return 1;
    }

    for (int i = 0; i < s->numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * sampleRoot * sampleRoot);
        generateJitteredSampleSet(s->squareSamples + offset, sampleRoot, NULL);
    }
    log_info("Done generating square samples.");

    /* Set up OpenCL buffer reference to square sample memory */
    s->squareSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, s->squareSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        return 1;
    }

    log_info("Generating disk samples...");
    /* Allocate and populate disk sample sets */
    s->diskSamples = malloc(samplesSize);
    if (!s->diskSamples) {
        log_error("Could not allocate %ld bytes of memory for disk samples", samplesSize);
        return 1;
    }

    for (int i = 0; i < s->numSampleSets; i++) {
        // Offset in number of floats for this set
        size_t offset = i * (2 * sampleRoot * sampleRoot);
        generateJitteredSampleSet(s->diskSamples + offset, sampleRoot, mapToUnitDisk);
    }
    log_info("Done generating disk samples.");

    /* Set up OpenCL buffer reference to disk sample memory */
    s->diskSampleBuf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR|CL_MEM_READ_ONLY,
            samplesSize, s->diskSamples, &ret);
    if (ret) {
        log_error("Could not create square sample buffer, ret %d", ret);
        return 1;
    }

    return 0;
}

static inline void rotateHeading(cl_float angle)
{
    programState.heading.x = cos(angle) * programState.heading.x -
                             sin(angle) * programState.heading.z;
    programState.heading.z = sin(angle) * programState.heading.x +
                             cos(angle) * programState.heading.z;

    normalize(&programState.heading);
    markStateDirty();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
        return;

    if (action == GLFW_PRESS)
    {
        SET_BIT(button_mask, MOUSE_PRESSED);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &cursorX, &cursorY);
    } else {
        CLEAR_BIT(button_mask, MOUSE_PRESSED);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
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

        markStateDirty();
        restartRendering();
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
#define PRESS(KEY) (key == KEY && (action == GLFW_PRESS || action == GLFW_REPEAT))
#define SHIFT (mods & GLFW_MOD_SHIFT)

#define QUIT            (PRESS(GLFW_KEY_ESCAPE) || PRESS(GLFW_KEY_Q))
#define MOVE_BACKWARD   (PRESS(GLFW_KEY_S))
#define MOVE_RIGHT      (PRESS(GLFW_KEY_D))
#define MOVE_FORWARD    (PRESS(GLFW_KEY_W))
#define MOVE_LEFT       (PRESS(GLFW_KEY_A))
#define DECREASE_DEPTH  (PRESS(GLFW_KEY_MINUS))
#define INCREASE_DEPTH  (PRESS(GLFW_KEY_EQUAL) && SHIFT)
#define DECREASE_RADIUS (PRESS(GLFW_KEY_R) && (!SHIFT))
#define INCREASE_RADIUS (PRESS(GLFW_KEY_R) && SHIFT)
#define TOGGLE_OVERLAY  (PRESS(GLFW_KEY_O))
#define DECREASE_SAMPLE_ROOT (PRESS(GLFW_KEY_T) && (!SHIFT))
#define INCREASE_SAMPLE_ROOT (PRESS(GLFW_KEY_T) && SHIFT)

    if (action == GLFW_PRESS) {
        SET_BIT(button_mask, KB_PRESSED);
    } else if (action == GLFW_RELEASE) {
        CLEAR_BIT(button_mask, KB_PRESSED);
    }

    if (DECREASE_SAMPLE_ROOT && config.sampleRoot > 1) {
        config.sampleRoot--;
        int ret = setup_samples(&samples, config.sampleRoot, &config, context);
        if (ret) {
            log_error("Could not set up samples");
            exit(1);
        }
        restartRendering();
        markConfigDirty();
    }

    if (INCREASE_SAMPLE_ROOT && config.sampleRoot < MAX_SAMPLE_ROOT) {
        config.sampleRoot++;
        int ret = setup_samples(&samples, config.sampleRoot, &config, context);
        if (ret) {
            log_error("Could not set up samples");
            exit(1);
        }
        restartRendering();
        markConfigDirty();
    }

    if (TOGGLE_OVERLAY) {
        programState.show_overlay = !programState.show_overlay;
        markStateDirty();
        log_info("Toggled overlay state, %d", programState.show_overlay);
    }

    if (QUIT)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (DECREASE_DEPTH && config.traceDepth > 0) {
        config.traceDepth = config.traceDepth == 0 ? 0 : config.traceDepth - 1;
        restartRendering();
        markConfigDirty();
    }

    if (INCREASE_DEPTH) {
        config.traceDepth++;
        restartRendering();
        markConfigDirty();
    }

    if (DECREASE_RADIUS && programState.lens_radius > 0.0) {
        programState.lens_radius = MAXF(programState.lens_radius - 0.01, 0.0);
        restartRendering();
        markStateDirty();
    }

    if (INCREASE_RADIUS) {
        programState.lens_radius += 0.01;
        restartRendering();
        markStateDirty();
    }

    // Movement keys translate the position vector based on the heading
    float vel = 0.2;
    float headingX = 0.0;
    float headingZ = 0.0;

    if (MOVE_FORWARD) {
        headingX = vel * programState.heading.x;
        headingZ = vel * programState.heading.z;
    }

    if (MOVE_BACKWARD) {
        headingX = -1 * vel * programState.heading.x;
        headingZ = -1 * vel * programState.heading.z;
    }

    if (MOVE_RIGHT) {
        headingX = -1 * vel * programState.heading.z;
        headingZ = vel * programState.heading.x;
    }

    if (MOVE_LEFT) {
        headingX = vel * programState.heading.z;
        headingZ = -1 * vel * programState.heading.x;
    }

    if (headingX != 0 || headingZ != 0) {
        programState.position.x += headingX;
        programState.position.z += headingZ;
        restartRendering();
        markStateDirty();
    }
}

int main(int argc, char **argv)
{
    cl_platform_id platform_id = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_int ret = -1;
    glResources res;

    processArgs(argc, argv, &config);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(config.width, config.height, "t2",
                                          NULL, NULL);
    if (!window) {
        log_error("Could not create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        log_error("GLEW initialization failed: %s", glewGetErrorString(err));
        exit(1);
    }

    logVersionInfo();

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Set swap interval */
    glfwSwapInterval(1);

    /* OpenCL initialization */

    /* Choose an OpenCL platform and create a context */
    platform_id = choosePlatform();
    logPlatformInfo(platform_id);
    context = createOpenCLContext();

    /* Choose an OpenCL device */
    cl_device_id device_id = chooseOpenCLDevice(platform_id, context);
    logDeviceInfo(device_id);

    log_info("Loading and building OpenCL kernel");

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

    log_info("Setting up textures and GLSL shaders");

    /* Set up GLSL shaders */
    ret = shader_setup(&res);

    /* Create rendering texture buffers */
    res.readTexture = make_texture(config.width, config.height);
    cl_mem texmemRead = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D,
            0, res.readTexture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 1, ret %d", ret);
        exit(1);
    }

    res.writeTexture = make_texture(config.width, config.height);
    cl_mem texmemWrite = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D,
            0, res.writeTexture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture 2, ret %d", ret);
        exit(1);
    }

    // Perform initial sample allocation/generation
    ret = setup_samples(&samples, config.sampleRoot, &config, context);
    if (ret) {
        log_error("Could not set up samples");
        exit(1);
    }

    /* Set up OpenCL buffer reference to configuration */
    configBuf = clCreateBuffer(context, CL_MEM_READ_ONLY,
            sizeof(struct configuration), NULL, &ret);
    if (ret) {
        log_error("Could not create configuration buffer, ret %d", ret);
        exit(1);
    }

    /* Set up OpenCL buffer for program state */
    stateBuf = clCreateBuffer(context, CL_MEM_READ_ONLY,
            sizeof(struct state), NULL, &ret);
    if (ret) {
        log_error("Could not create configuration buffer, ret %d", ret);
        exit(1);
    }

    ret = initialize_overlay(&config);
    if (ret) {
        log_error("Could not initialize overlay");
        exit(1);
    }

    log_info("Ready.");

    struct timeval start;
    size_t global_work_size[2] = { config.width, config.height };
    cl_uint batchSize = 0;

    ret  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &configBuf);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &stateBuf);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &texmemRead);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &texmemWrite);

    if (ret) {
        log_error("Could not set kernel argument, ret %d", ret);
        exit(1);
    }

    while (!glfwWindowShouldClose(window))
    {
        if (ANY_PRESSED && (oldBatchSize == -1)) {
            log_debug("Lowering batch size to 1");
            oldBatchSize = config.batchSize;
            config.batchSize = 1;
        } else if (NONE_PRESSED && oldBatchSize != -1) {
            log_debug("Restoring batch size to %d", oldBatchSize);
            config.batchSize = oldBatchSize;
            oldBatchSize = -1;
        }

        if (programState.sampleNum < (config.sampleRoot * config.sampleRoot)) {
            programState.last_frame_time = -1;

            if (programState.sampleNum == 0)
                gettimeofday(&start, NULL);

            /* Before we begin collecting a new sample, copy the old
               write buffer to the read buffer. This is critical because
               the kernel reads the image data and averages new sample
               data with it before writing back out, so we need to read
               this time what we wrote last time. Since OpenCL doesn't
               support read-write images, we have to have two: one to
               read, one to write, and some code to copy between them at
               the right time (now). */
            copyTexture(res.fbo, res.writeTexture, res.readTexture,
                    config.width, config.height);

            /* Determine the number of samples in this batch */
            batchSize = MINF(config.batchSize,
                    config.sampleRoot * config.sampleRoot - programState.sampleNum);

            /* Set OpenCL Kernel Parameters */
            ret = 0;
            ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &samples.squareSampleBuf);
            ret |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &samples.diskSampleBuf);
            ret |= clSetKernelArg(kernel, 6, sizeof(cl_int), &samples.numSampleSets);
            ret |= clSetKernelArg(kernel, 7, sizeof(batchSize), &batchSize);

            if (ret) {
                log_error("Could not set kernel argument, ret %d", ret);
                exit(1);
            }

            /* Update dirty structs */
            updateConfigBuffer();
            updateStateBuffer();

            /* Acquire OpenGL objects */
            ret = clEnqueueAcquireGLObjects(command_queue, 1, &texmemRead, 0, NULL, NULL);
            ret |= clEnqueueAcquireGLObjects(command_queue, 1, &texmemWrite, 0, NULL, NULL);
            if (ret) {
                log_error("Could not issue OpenCL commands, ret %d", ret);
                exit(1);
            }

            /* Execute OpenCL Kernel */
            ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size,
                    NULL, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue task");
                exit(1);
            }

            // Before returning the objects to OpenGL, we sync to make sure OpenCL is done.
            clFinish(command_queue);

            ret = clEnqueueReleaseGLObjects(command_queue, 1, &texmemRead, 0, NULL, NULL);
            ret |= clEnqueueReleaseGLObjects(command_queue, 1, &texmemWrite, 0, NULL, NULL);
            if (ret) {
                log_error("Could not enqueue GL object releases, ret %d", ret);
                exit(1);
            }

            programState.sampleNum += batchSize;
            markStateDirty();

            if (programState.sampleNum == config.sampleRoot * config.sampleRoot) {
                struct timeval stop;
                gettimeofday(&stop, NULL);
                struct timeval diff;
                timevalDiff(&start, &stop, &diff);
                float secs = ((float)diff.tv_sec) + ((float) diff.tv_usec / 1000000.0);
                programState.last_frame_time = secs;
            }
        }

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

        if (programState.show_overlay)
            render_overlay(&config, &programState);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

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
