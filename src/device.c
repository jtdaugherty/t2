
#include <stdlib.h>

#include <t2/device.h>
#include <t2/logging.h>

#define MAX_DEVICES 10

cl_device_id chooseDevice(cl_platform_id platform_id) {
    cl_uint ret_num;
    cl_device_id device_ids[MAX_DEVICES];

    cl_int ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAX_DEVICES, device_ids, &ret_num);
    if (ret) {
        log_error("Could not get device IDs, ret %d", ret);
        return NULL;
    }

    log_info("Found %d GPU devices", ret_num);

    if (ret_num < 1) {
        log_error("Failed to find a qualifying device!");
        return NULL;
    }

    log_info("Using first GPU device");
    return device_ids[0];
}
