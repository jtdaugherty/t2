
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>

#include <t2/util.h>
#include <t2/logging.h>

#define MAX_SOURCE_SIZE 0x20000
#define MAX_LOG_SIZE    0x10000

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
    ret = clBuildProgram(program, 1, &device_id, "-Werror -Icl -Iinclude", NULL, NULL);
    if (ret) {
        log_error("Error building %s", path);

        char build_log[MAX_LOG_SIZE];
        size_t actual_size;

        ret = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, MAX_LOG_SIZE, build_log, &actual_size);
        if (ret) {
            log_error("Error getting build information, ret %d", ret);
            return NULL;
        } else {
            log_error("Build log: %s", build_log);
        }

        return NULL;
    } else {
        return program;
    }
}

void timevalDiff(struct timeval *start,
                 struct timeval *stop,
                 struct timeval *diff)
{
    if (stop->tv_sec == start->tv_sec + 1)
        diff->tv_sec = 0;
    else
        diff->tv_sec = stop->tv_sec - start->tv_sec;

    if (stop->tv_sec == start->tv_sec)
        diff->tv_usec = stop->tv_usec - start->tv_usec;
    else
        diff->tv_usec = stop->tv_usec + (1000000L - start->tv_usec);

    if (diff->tv_usec > 1000000L) {
        diff->tv_usec -= 1000000L;
        diff->tv_sec += 1;
    }
}
