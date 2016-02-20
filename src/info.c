
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <t2/info.h>
#include <t2/logging.h>
#include <t2/version.h>
#include <t2/text.h>

static const char * localMemTypeStr(cl_device_local_mem_type type)
{
    if (type == CL_LOCAL) {
        return "local";
    } else if (type == CL_GLOBAL) {
        return "global";
    } else {
        return "<unknown local memory type>";
    }
}

static const char * cacheTypeStr(cl_device_mem_cache_type type)
{
    if (type == CL_NONE) {
        return "None";
    } else if (type == CL_READ_ONLY_CACHE) {
        return "Read-only";
    } else if (type == CL_READ_WRITE_CACHE) {
        return "Read-write";
    } else {
        return "<unknown cache type>";
    }
}

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
    char openclCVersion[1024] = {0};
    size_t maxWorkGroupSize = 0;
    size_t maxWorkItemDimensions = 0;
    size_t deviceAddressBits = 0;
    cl_ulong globalMemCacheSize = 0;
    cl_device_mem_cache_type globalMemCacheType = CL_NONE;
    cl_ulong globalMemSize = 0;
    cl_ulong localMemSize = 0;
    cl_bool unified;
    cl_device_local_mem_type localMemType;
    cl_uint maxClockFreq;
    cl_uint maxComputeUnits;
    cl_ulong maxConstantBufferSize;
    cl_ulong maxMemoryAllocationSize;
    cl_uint baseAddrAlignment;
    cl_uint minDataAlignment;

    ret = clGetDeviceInfo(device_id, CL_DEVICE_VENDOR, sizeof(vendorName), vendorName, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_OPENCL_C_VERSION, sizeof(openclCVersion), openclCVersion, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize),
            &maxWorkGroupSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxWorkItemDimensions),
            &maxWorkItemDimensions, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_ADDRESS_BITS, sizeof(deviceAddressBits),
            &deviceAddressBits, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(globalMemCacheSize),
            &globalMemCacheSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(globalMemCacheType),
            &globalMemCacheType, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(globalMemSize),
            &globalMemSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMemSize),
            &localMemSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(unified),
            &unified, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(localMemType),
            &localMemType, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockFreq),
            &maxClockFreq, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits),
            &maxComputeUnits, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(maxConstantBufferSize),
            &maxConstantBufferSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxMemoryAllocationSize),
            &maxMemoryAllocationSize, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(baseAddrAlignment),
            &baseAddrAlignment, NULL);
    ret |= clGetDeviceInfo(device_id, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(minDataAlignment),
            &minDataAlignment, NULL);
    if (ret) {
        log_error("Failed to get device info, ret %d", ret);
        return ret;
    }

    // General info
    log_info("OpenCL device details:");
    log_info("  Name: %s", deviceName);
    log_info("  Vendor: %s", vendorName);
    log_info("  OpenCL C version: %s", openclCVersion);
    log_info("  Device address size: %ld bits", deviceAddressBits);
    log_info("  Memory address alignment: %u bits", baseAddrAlignment);
    log_info("  Minimum data type alignment: %u bytes", minDataAlignment);
    log_info("  Maximum work item dimensions: %ld", maxWorkItemDimensions);
    log_info("  Maximum work group size: %ld work items", maxWorkGroupSize);
    log_info("  Maximum compute units: %u", maxComputeUnits);
    log_info("  Maximum clock frequency: %u MHz", maxClockFreq);
    log_info("  Unified host-device memory subsystem: %s", unified ? "yes" : "no");
    log_info("  Maximum memory allocation size: %lld bytes", maxMemoryAllocationSize);

    // Global memory
    log_info("  Global memory size: %lld bytes", globalMemSize);
    log_info("  Global memory cache type: %s", cacheTypeStr(globalMemCacheType));
    if (globalMemCacheType != CL_NONE)
        log_info("  Global memory cache size: %llu bytes", globalMemCacheSize);

    // Local & constant memory
    log_info("  Local memory size: %lld bytes", localMemSize);
    log_info("  Local memory type: %s", localMemTypeStr(localMemType));
    log_info("  Maximum constant buffer size: %lld bytes", maxConstantBufferSize);

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
