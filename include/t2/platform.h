
#ifndef T2_PLATFORM_H
#define T2_PLATFORM_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

cl_platform_id choosePlatform();

#endif
