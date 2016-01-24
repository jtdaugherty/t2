
#ifndef T2_INFO_H
#define T2_INFO_H

#include <t2/opencl_setup.h>

cl_int logDeviceInfo(cl_device_id device_id);
cl_int logPlatformInfo(cl_platform_id platform_id);

#endif
