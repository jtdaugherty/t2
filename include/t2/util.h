
#ifndef T2_UTIL_H
#define T2_UTIL_H

#include <t2/opencl_setup.h>

cl_program readAndBuildProgram(cl_context context, cl_device_id device_id, const char *path, int *res);

#endif
