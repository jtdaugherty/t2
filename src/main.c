#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x20000)

#define MAX_DEVICES 10

// Log levels
#define LOG_ERROR    (1 << 0)
#define LOG_WARN     (1 << 1)
#define LOG_INFO     (1 << 2)
#define LOG_DEBUG    (1 << 3)

int global_log_level = LOG_DEBUG;

#define do_log(level, level_str, fmt, ...) do { \
    if (level <= global_log_level) { \
        fprintf(stderr, "[" level_str "] "); \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
    } while (0)

#define log_error(fmt, ...) do_log(LOG_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  do_log(LOG_INFO,  "INFO",  fmt, ##__VA_ARGS__)

cl_program readAndBuildProgram(cl_context context, cl_device_id device_id, const char *path, int *res) {
    int ret;
	char *source_str;
	size_t source_size;
    FILE *fp;
    cl_program program;

	/* Load the source code containing the kernel*/
	fp = fopen(path, "r");
	if (!fp) {
		log_error("Failed to load kernel.");
		exit(1);
	}

	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
    if (source_size == 0) {
        log_error("Error reading file");
        free(source_str);
        return NULL;
    }

	/* Create Kernel Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
			(const size_t *)&source_size, &ret);
    if (ret) {
        log_error("Could not create program with source, ret %d", ret);
        return NULL;
    }

	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret) {
        log_error("Error building %s", path);
        return NULL;
    } else {
        return program;
    }
}
 
int main()
{
	cl_device_id device_ids[MAX_DEVICES];
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	char string[MEM_SIZE];

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    log_info("Number of platforms: %d", ret_num_platforms);

	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, MAX_DEVICES, device_ids, &ret_num_devices);
    log_info("Number of CPU devices: %d", ret_num_devices);

	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAX_DEVICES, device_ids, &ret_num_devices);
    log_info("Number of GPU devices: %d", ret_num_devices);

	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_ids[0], NULL, NULL, &ret);

	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_ids[0], 0, &ret);

	/* Create Memory Buffer */
	memobj = clCreateBuffer(context, CL_MEM_READ_WRITE,MEM_SIZE * sizeof(char), NULL, &ret);

	/* Create Kernel Program from the source */
	program = readAndBuildProgram(context, device_ids[0], "cl/t2.cl", &ret);
    if (!program) {
        log_error("readAndBuildProgram failed, ret %d", ret);
        exit(1);
    }

	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, "t2main", &ret);
    if (ret) {
        printf("Could not create kernel!\n");
        exit(1);
    }

	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);

	/* Execute OpenCL Kernel */
	ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);

	/* Do a blocking read to copy results from the memory buffer */
	ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
			MEM_SIZE * sizeof(char), string, 0, NULL, NULL);

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

	return 0;
}
