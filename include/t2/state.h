
#ifndef T2_STATE_H
#define T2_STATE_H

#ifndef __OPENCL_C_VERSION__
#include <t2/opencl_setup.h>
#endif

struct state {
#ifdef __OPENCL_C_VERSION__
    float3 position;
    float3 heading;
    float lens_radius;
    uint sampleNum;
    uint show_overlay;
    float last_frame_time;
#else
    cl_float3 position;
    cl_float3 heading;
    cl_float lens_radius;
    cl_uint sampleNum;
    cl_uint show_overlay;
    cl_float last_frame_time;
#endif
};

#endif
