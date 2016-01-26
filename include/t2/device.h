
#ifndef T2_DEVICE_H
#define T2_DEVICE_H

#include <t2/opencl_setup.h>

cl_context createOpenCLContext();
cl_device_id chooseOpenCLDevice(cl_platform_id platform_id, cl_context context);

#endif
