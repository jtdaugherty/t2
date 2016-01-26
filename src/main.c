
#include <GL/glew.h>

#include <OpenGL/gl.h>
#include <OpenGL/CGLCurrent.h>
#include <GLFW/glfw3.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
#include <t2/util.h>
#include <t2/opencl_setup.h>
#include <t2/shader_setup.h>
#include <t2/texture.h>
 
int global_log_level = LOG_DEBUG;

float width_zoom = 1.0;
float height_zoom = 1.0;

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
#define PRESS(KEY) (key == KEY && action == GLFW_PRESS)
#define QUIT_KEY (PRESS(GLFW_KEY_ESCAPE) || PRESS(GLFW_KEY_Q))

    if (QUIT_KEY)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void clNotify(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
    log_error("%s", errinfo);
}

int main()
{
    cl_platform_id platform_id = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_int ret = -1;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    log_debug("GLFW initialized");

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    log_debug("Set GLFW window hints");

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(640, 480, "t2", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    log_debug("GLFW window created");

    glfwMakeContextCurrent(window);

    log_debug("Made window OpenGL context current");

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        log_error("GLEW initialization failed: %s", glewGetErrorString(err));
        exit(1);
    }

    log_info("Using GLEW %s", glewGetString(GLEW_VERSION));
    
    glfwSetKeyCallback(window, key_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* OpenCL initialization */

    /* Choose an OpenCL platform */
    platform_id = choosePlatform();
    if (!platform_id) {
        exit(1);
    } else {
        logPlatformInfo(platform_id);
    }

    CGLContextObj cglContext = CGLGetCurrentContext();
    if (!cglContext) {
        log_error("Could not get CGLContext");
        exit(1);
    }

    CGLShareGroupObj cglShareGroup = CGLGetShareGroup(cglContext);
    if (!cglShareGroup) {
        log_error("Could not get share group");
        exit(1);
    }

    cl_context_properties clProperties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)cglShareGroup, 0
    };

    /* Create OpenCL context using share group so we can do efficient
    rendering from OpenCL kernel */
    cl_context context = clCreateContext(clProperties, 0, NULL, clNotify, NULL, &ret);
    if (ret) {
        log_error("Could not create context, ret %d", ret);
        exit(1);
    }

    log_debug("Created OpenCL context");

    /* Get device ID from context */
    size_t returned;
    cl_device_id device_ids[10];
    ret = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(device_ids), device_ids, &returned);
    if (ret) {
        log_error("Could not get devices from context, ret %d", ret);
        exit(1);
    }

    int num_devices = returned / sizeof(cl_device_id);
    log_info("Devices found in context: %d", num_devices);

    if (num_devices < 1) {
        log_error("No suitable devices available in context, exiting");
        exit(1);
    }

    /* Create Command Queue */
    cl_device_id device_id = device_ids[0];
    logDeviceInfo(device_id);

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret) {
        log_error("Could not create command queue, ret %d", ret);
        exit(1);
    }

    log_debug("Created OpenCL command queue");

    /* Create Kernel Program from the source */
    program = readAndBuildProgram(context, device_id, "cl/t2.cl", &ret);
    if (!program) {
        log_error("readAndBuildProgram failed, ret %d", ret);
        exit(1);
    }

    log_debug("Read OpenCL kernel program source");

    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "t2main", &ret);
    if (ret) {
        log_error("Could not create kernel!\n");
        exit(1);
    }

    log_debug("Created kernel");

    /* Call window size callback at least once */
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glfwSwapInterval(1);

    resources res;

    ret = shader_setup(&res);
    if (ret) {
        log_error("Error creating shader program, ret %d", ret);
        exit(1);
    }

    log_debug("Created shader program");

    GLuint texture = make_texture(width, height);

    log_debug("Created texture");

    GLint position_attribute = glGetAttribLocation(res.shader_program, "position");
    if (position_attribute == -1) {
        log_error("Could not get position attribute location");
        exit(1);
    }

    GLint texture_uniform = glGetUniformLocation(res.shader_program, "texture");
    if (texture_uniform == -1) {
        log_error("Could not get texture uniform location");
        exit(1);
    }

    log_debug("Finished setting attribute/uniform locations");

    cl_mem texmem = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture, ret %d", ret);
        exit(1);
    }
    
    log_debug("Created shared OpenCL/OpenGL texture");

    log_debug("Entering graphics loop");

    float amt = 1.0;
    int dir = 0;

    struct timeval start;
    gettimeofday(&start, NULL);
    float frames = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Set OpenCL Kernel Parameters */
        ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&texmem);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clSetKernelArg(kernel, 1, sizeof(float), (void *)&amt);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clEnqueueAcquireGLObjects(command_queue, 1, &texmem, 0, NULL, NULL);
        if (ret) {
            log_error("Could not enqueue GL object acquisition, ret %d", ret);
            exit(1);
        }

        /* Execute OpenCL Kernel */
        size_t global_work_size[2] = { width, height };
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);
        if (ret) {
            log_error("Could not enqueue task\n");
            exit(1);
        }

        // Before returning the objects to OpenGL, we sync to make sure OpenCL is done.
        ret = clEnqueueReleaseGLObjects(command_queue, 1, &texmem, 0, NULL, NULL);
        if (ret) {
            log_error("Could not enqueue GL object acquisition, ret %d", ret);
            exit(1);
        }

        if (dir == 0) {
            amt -= 0.01;
            if (amt < 0) {
                amt = 0;
                dir = 1;
            }
        } else {
            amt += 0.01;
            if (amt > 1) {
                amt = 1;
                dir = 0;
            }
        }

        glPixelZoom(width_zoom, height_zoom);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(res.shader_program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(texture_uniform, 0);

        glBindBuffer(GL_ARRAY_BUFFER, res.vertex_buffer);
        glVertexAttribPointer(
                position_attribute,               /* attribute */
                2,                                /* size */
                GL_FLOAT,                         /* type */
                GL_FALSE,                         /* normalized? */
                sizeof(GLfloat)*2,                /* stride */
                (void*)0                          /* array buffer offset */
                );
        glEnableVertexAttribArray(position_attribute);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.element_buffer);
        glDrawElements(
                GL_TRIANGLE_STRIP,  /* mode */
                4,                  /* count */
                GL_UNSIGNED_SHORT,  /* type */
                (void*)0            /* element array buffer offset */
                );

        glDisableVertexAttribArray(position_attribute);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        if (frames >= 200.0) {
            struct timeval now;
            gettimeofday(&now, NULL);

            // Measure difference, compute average frame rate
            float fps = (float) frames / (float) (now.tv_sec - start.tv_sec);
            log_info("FPS: %f", fps);

            start = now;
            frames = 0.0;
        } else {
            frames += 1.0;
        }
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
