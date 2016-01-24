
#include <sys/stat.h>
#include <stdio.h>

#include <t2/util.h>
#include <t2/logging.h>

#define MAX_SOURCE_SIZE 0x20000

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
