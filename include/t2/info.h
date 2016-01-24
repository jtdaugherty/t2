
#ifndef T2_INFO_H
#define T2_INFO_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

cl_int logDeviceInfo(cl_device_id device_id);
cl_int logPlatformInfo(cl_platform_id platform_id);

#endif
