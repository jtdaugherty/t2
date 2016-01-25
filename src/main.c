
#include <GLFW/glfw3.h>

#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
#include <t2/util.h>
#include <t2/opencl_setup.h>
 
#define MEM_SIZE (128)

int global_log_level = LOG_DEBUG;

static void key_callback(GLFWwindow* window, int key, int scancode,
        int action, int mods)
{
#define PRESS(KEY) (key == KEY && action == GLFW_PRESS)
#define QUIT_KEY (PRESS(GLFW_KEY_ESCAPE) || PRESS(GLFW_KEY_Q))

    if (QUIT_KEY)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main()
{
    cl_context context = NULL;
    cl_device_id device_id = NULL;
    cl_platform_id platform_id = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem memobj = NULL;
    cl_int ret = -1;
    char string[MEM_SIZE];

    /* Choose an OpenCL platform */
    platform_id = choosePlatform();
    if (!platform_id) {
        exit(1);
    } else {
        logPlatformInfo(platform_id);
    }

    /* Choose an OpenCL device */
    device_id = chooseDevice(platform_id);
    if (!device_id) {
        exit(1);
    } else {
        logDeviceInfo(device_id);
    }

    /* Create OpenCL context */
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (ret) {
        log_error("Could not create context\n");
        exit(1);
    }

    /* Create Command Queue */
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (ret) {
        log_error("Could not create command queue\n");
        exit(1);
    }

    /* Create Memory Buffer */
    memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);
    if (ret) {
        log_error("Could not create memory buffer\n");
        exit(1);
    }

    /* Create Kernel Program from the source */
    program = readAndBuildProgram(context, device_id, "cl/t2.cl", &ret);
    if (!program) {
        log_error("readAndBuildProgram failed, ret %d", ret);
        exit(1);
    }

    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "t2main", &ret);
    if (ret) {
        log_error("Could not create kernel!\n");
        exit(1);
    }

    /* Set OpenCL Kernel Parameters */
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);
    if (ret) {
        log_error("Could not set kernel argument\n");
        exit(1);
    }

    /* Execute OpenCL Kernel */
    ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    if (ret) {
        log_error("Could not enqueue task\n");
        exit(1);
    }

    /* Do a blocking read to copy results from the memory buffer */
    ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
            MEM_SIZE * sizeof(char), string, 0, NULL, NULL);
    if (ret) {
        log_error("Could not enqueue buffer read");
        exit(1);
    }

    /* Display Result */
    puts(string);

    /* Finalization */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(memobj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    /* Mess around with glfw */
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set window hints */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "t2", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    unsigned char *buf = malloc(640 * 480 * 3);

    for (int i = 0; i < 640; i++) {
        for (int j = 0; j < 480; j++) {
            int base = ((i * 480) + j) * 3;
            buf[base] = 255 * ((float) i/640.0);
            buf[base + 1] = 0; // * ((float) j/480.0);
            buf[base + 2] = 0;
        }
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glDrawPixels(640, 480, GL_RGB, GL_UNSIGNED_BYTE, buf);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
