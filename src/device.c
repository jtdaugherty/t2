
#include <stdlib.h>

#include <OpenGL/CGLCurrent.h>

#include <t2/device.h>
#include <t2/logging.h>

#define MAX_DEVICES 10

static void clNotify(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
    log_error("%s", errinfo);
}

cl_context createOpenCLContext() {
    int ret;

    CGLContextObj cglContext = CGLGetCurrentContext();
    if (!cglContext) {
        log_error("Could not get CGLContext");
        exit(1);
    }

    CGLShareGroupObj cglShareGroup = CGLGetShareGroup(cglContext);
    if (!cglShareGroup) {
        log_error("Could not get share group");
        exit(1);
    }

    cl_context_properties clProperties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)cglShareGroup, 0
    };

    /* Create OpenCL context using share group so we can do efficient
    rendering from OpenCL kernel */
    cl_context context = clCreateContext(clProperties, 0, NULL, clNotify, NULL, &ret);
    if (ret) {
        log_error("Could not create context, ret %d", ret);
        exit(1);
    }

    return context;
}

cl_device_id chooseOpenCLDevice(cl_platform_id platform_id, cl_context context)
{
    size_t returned;
    int ret;
    cl_device_id device_ids[10];

    ret = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(device_ids), device_ids, &returned);
    if (ret) {
        log_error("Could not get devices from context, ret %d", ret);
        exit(1);
    }

    int num_devices = returned / sizeof(cl_device_id);
    log_info("Devices found in OpenCL context: %d", num_devices);

    if (num_devices < 1) {
        log_error("No suitable devices available in context, exiting");
        exit(1);
    }

    /* Create Command Queue */
    return device_ids[0];
}
