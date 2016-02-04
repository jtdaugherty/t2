
#ifndef T2_STATE_H
#define T2_STATE_H

#include <t2/opencl_setup.h>

struct state {
    // Position vector
    cl_float position[3];

    // Heading vector
    cl_float heading[3];

    // Camera lens radius
    cl_float lens_radius;

    // Current sample index being rendered
    cl_uint sampleIdx;

    // Whether to render the overlay
    cl_uint show_overlay;

    // The rendering time of the most recently completed frame
    cl_float last_frame_time;
};

#endif
