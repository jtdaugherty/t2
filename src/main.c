
#include <GLFW/glfw3.h>

#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
#include <t2/util.h>
#include <t2/opencl_setup.h>
 
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

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);

    width_zoom = fbW / ((float) width);
    height_zoom = fbH / ((float) height);

    log_info("Window size: %dx%d", width, height);
    log_info("Framebuffer size: %dx%d", fbW, fbH);
    log_info("Pixel zoom factors: %f, %f", width_zoom, height_zoom);

    glViewport(0, 0, fbW, fbH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, fbW, 0, fbH, -1, 1);

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
    cl_int ret = -1;

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

    /* Create an OpenCL image */
    cl_image_format bufFmt = {
        .image_channel_order = CL_RGBA,
        .image_channel_data_type = CL_UNSIGNED_INT8
    };

    cl_image_desc imgDesc = {
        .image_type = CL_MEM_OBJECT_IMAGE2D,
        .image_width = 640,
        .image_height = 480
    };

    cl_mem imgBuf = clCreateImage(context, CL_MEM_READ_WRITE, &bufFmt, &imgDesc, NULL, &ret);
    if (ret || !imgBuf) {
        log_error("Could not create OpenCL image, ret %d", ret);
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
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&imgBuf);
    if (ret) {
        log_error("Could not set kernel argument, ret %d", ret);
        exit(1);
    }

    /* Execute OpenCL Kernel */
    ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    if (ret) {
        log_error("Could not enqueue task\n");
        exit(1);
    }

    const size_t origin[3] = { 0, 0, 0 };
    const size_t region[3] = { 640, 480, 1 };
    size_t rowPitch;

    /* Do a blocking read to copy results from the memory buffer */
    unsigned char *buf = clEnqueueMapImage(command_queue, imgBuf, CL_TRUE, CL_MAP_READ,
            origin, region, &rowPitch, NULL, 0, NULL, NULL, &ret);
    if (ret) {
        log_error("Could not enqueue image mapping, ret %d", ret);
        exit(1);
    }

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

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    /* Call window size callback at least once */
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    window_size_callback(window, width, height);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glPixelZoom(width_zoom, height_zoom);

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawPixels(640, 480, GL_RGBA, GL_UNSIGNED_BYTE, buf);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    /* Finalization */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(imgBuf);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    return 0;
}
