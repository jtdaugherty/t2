
#include <t2/logging.h>
#include <t2/info.h>
#include <t2/platform.h>
#include <t2/device.h>
 
#include <sys/stat.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
 
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x20000)

int global_log_level = LOG_DEBUG;

cl_program readAndBuildProgram(cl_context context, cl_device_id device_id, const char *path, int *res) {
    int ret;
	char *source_str;
	size_t source_size;
    FILE *fp;
    cl_program program;
    struct stat st;

    if (stat(path, &st)) {
        log_error("Could not get file size for %s", path);
        return NULL;
    }

    if (st.st_size > MAX_SOURCE_SIZE) {
        log_error("File size %lld exceeds allowed size %d for file %s", st.st_size, MAX_SOURCE_SIZE, path);
        return NULL;
    }

	/* Load the source code containing the kernel*/
	fp = fopen(path, "r");
	if (!fp) {
		log_error("Failed to load kernel.");
		exit(1);
	}

	source_str = (char*)malloc(st.st_size);
	source_size = fread(source_str, 1, st.st_size, fp);
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
	cl_context context = NULL;
    cl_device_id device_id;
    cl_platform_id platform_id;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_mem memobj = NULL;
	cl_int ret;
	char string[MEM_SIZE];

    /* Choose an OpenCL platform */
    platform_id = choosePlatform();
    if (!platform_id) {
        exit(1);
    } else {
        logPlatformInfo(platform_id);
    }

    device_id = chooseDevice(platform_id);
    if (!device_id) {
        exit(1);
    } else {
        logDeviceInfo(device_id);
    }

	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	/* Create Memory Buffer */
	memobj = clCreateBuffer(context, CL_MEM_READ_WRITE,MEM_SIZE * sizeof(char), NULL, &ret);

	/* Create Kernel Program from the source */
	program = readAndBuildProgram(context, device_id, "cl/t2.cl", &ret);
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
