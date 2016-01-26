
#include <t2/info.h>
#include <t2/logging.h>

cl_int logDeviceInfo(cl_device_id device_id) {
    char extension_list[1024];
    cl_int ret = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 1024, extension_list, NULL);
    if (ret) {
        log_error("Failed to get device info, ret %d", ret);
        return ret;
    }

    char vendorName[1024] = {0};
    char deviceName[1024] = {0};

    ret = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
    if (ret) {
        log_error("Failed to get device info, ret %d", ret);
        return ret;
    }

    log_info("Device details:");
    log_info("  Name: %s", deviceName);
    log_info("  Vendor: %s", vendorName);
    log_info("  Extensions: %s", extension_list);
    return 0;
}

cl_int logPlatformInfo(cl_platform_id platform_id) {
    char platform_profile[64];
    cl_int ret = clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 64, platform_profile, NULL);
    if (ret) {
        log_error("Failed to get platform profile, ret %d", ret);
        return ret;
    }

    log_info("Platform profile: %s", platform_profile);
    return 0;
}
