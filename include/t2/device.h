
#ifndef T2_DEVICE_H
#define T2_DEVICE_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

cl_device_id chooseDevice(cl_platform_id platform_id);

#endif
