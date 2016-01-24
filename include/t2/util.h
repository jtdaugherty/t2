
#ifndef T2_UTIL_H
#define T2_UTIL_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

cl_program readAndBuildProgram(cl_context context, cl_device_id device_id, const char *path, int *res);

#endif
