
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <t2/info.h>
#include <t2/logging.h>
#include <t2/version.h>
#include <t2/text.h>

cl_int logDeviceInfo(cl_device_id device_id)
{
    char extension_list[1024];
    cl_int ret = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 1024, extension_list, NULL);
    if (ret) {
        log_error("Failed to get device info, ret %d", ret);
        return ret;
    }

    char vendorName[1024] = {0};
    char deviceName[1024] = {0};
    size_t maxWorkGroupSize = 0;

    ret = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize),
            &maxWorkGroupSize, NULL);
    if (ret) {
        log_error("Failed to get device info, ret %d", ret);
        return ret;
    }

    log_info("OpenCL device details:");
    log_info("  Name: %s", deviceName);
    log_info("  Vendor: %s", vendorName);
    log_info("  Maximum work group size: %ld", maxWorkGroupSize);

    char *start = extension_list;
    char *found = NULL;
    char extName[128];
    int len = 0;

    log_info("  Extensions:");
    while ((found = strchr(start, ' ')) != NULL) {
        len = found - start;
        memcpy(extName, start, len);
        extName[len] = 0;
        log_info("    %s", extName);
        start = found + 1;
    }

    return 0;
}

cl_int logPlatformInfo(cl_platform_id platform_id)
{
    cl_int ret;

    char platform_profile[64];
    ret = clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 64, platform_profile, NULL);
    if (ret) {
        log_error("Failed to get platform profile, ret %d", ret);
        return ret;
    }

    char platform_version[64];
    ret = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, 64, platform_version, NULL);
    if (ret) {
        log_error("Failed to get platform version, ret %d", ret);
        return ret;
    }

    log_info("OpenCL platform:");
    log_info("  Profile: %s", platform_profile);
    log_info("  Version: %s", platform_version);
    return 0;
}

void logVersionInfo()
{
    int maj, min, rev;

    log_info("t2 version %s (commit %s)", T2_VERSION, T2_COMMIT);

    glfwGetVersion(&maj, &min, &rev);
    log_info("GLFW version: %d.%d.%d", maj, min, rev);

    log_info("GLEW version: %s", glewGetString(GLEW_VERSION));

    logTextSystemInfo();
}
