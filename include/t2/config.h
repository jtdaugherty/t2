
#ifndef T2_CONFIG_H
#define T2_CONFIG_H

#include <t2/opencl_setup.h>

struct configuration {
    // How deeply will reflective tracing go?
    cl_uint traceDepth;

    // If this is r, we take r*r samples per pixel
    int sampleRoot;

    // Window width
    int width;

    // Window height
    int height;

    // Log level (see t2/logging.h)
    int logLevel;

    // Batch size: number of samples per kernel invocation
    int batchSize;

    // Whether sampling is paused
    int paused;

    // Whether to run in fullscreen mode
    int fullScreen;
};

#endif
