
#include <GL/glew.h>

#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <sys/stat.h>
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

// Position vector
cl_float position[3] = { 0.0, 1.0, -10.0 };

// Heading vector
cl_float heading[3] = { 0.0, 0.0, 1.0 };

double cursorX;
double cursorY;

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
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
#define PRESS(KEY) (key == KEY && (action == GLFW_PRESS || action == GLFW_REPEAT))
#define QUIT_KEY (PRESS(GLFW_KEY_ESCAPE) || PRESS(GLFW_KEY_Q))
#define A_KEY (PRESS(GLFW_KEY_A))
#define D_KEY (PRESS(GLFW_KEY_D))
#define W_KEY (PRESS(GLFW_KEY_W))
#define S_KEY (PRESS(GLFW_KEY_S))

    if (QUIT_KEY)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Movement keys translate the position vector based on the heading
    float vel = 0.2;
    float headingX = 0.0;
    float headingZ = 0.0;

    if (W_KEY) {
        headingX = vel * heading[0];
        headingZ = vel * heading[2];
    }

    if (S_KEY) {
        headingX = -1 * vel * heading[0];
        headingZ = -1 * vel * heading[2];
    }

    if (D_KEY) {
        headingX = -1 * vel * heading[2];
        headingZ = vel * heading[0];
    }

    if (A_KEY) {
        headingX = vel * heading[2];
        headingZ = -1 * vel * heading[0];
    }

    position[0] += headingX;
    position[2] += headingZ;
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

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(640, 480, "t2", NULL, NULL);
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

    log_info("Using GLEW %s", glewGetString(GLEW_VERSION));
    
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

    GLuint texture = make_texture(width, height);
    cl_mem texmem = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, texture, &ret);
    if (ret) {
        log_error("Could not create shared OpenCL/OpenGL texture, ret %d", ret);
        exit(1);
    }
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Set OpenCL Kernel Parameters */
        ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&texmem);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clSetKernelArg(kernel, 1, sizeof(width), (void *)&width);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clSetKernelArg(kernel, 2, sizeof(height), (void *)&height);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clSetKernelArg(kernel, 3, sizeof(position), (void *)position);
        if (ret) {
            log_error("Could not set kernel argument, ret %d", ret);
            exit(1);
        }

        ret = clSetKernelArg(kernel, 4, sizeof(heading), (void *)heading);
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

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(res.shader_program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
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
