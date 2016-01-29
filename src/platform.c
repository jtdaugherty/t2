
#include <stdlib.h>

#include <t2/platform.h>
#include <t2/logging.h>

#define MAX_PLATFORMS 10

cl_platform_id choosePlatform() {
    cl_platform_id platform_ids[MAX_PLATFORMS];
    cl_uint ret_num;

    /* Get Platform and Device Info */
    cl_int ret = clGetPlatformIDs(MAX_PLATFORMS, platform_ids, &ret_num);
    if (ret) {
        log_error("Could not get platforms, ret %d", ret);
        exit(1);
    }

    if (ret_num < 1) {
        log_error("Could not find suitable platform");
        exit(1);
    }

    char *s = "";
    if (ret_num != 1)
        s = "s";

    log_info("Found %d platform%s, using the first one", ret_num, s);
    return platform_ids[0];
}
