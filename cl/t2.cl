
#include <t2/extensions.cl>
#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/scene.cl>
#include <t2/pinhole_camera.cl>
#include <t2/thinlens_camera.cl>
#include <t2/config.cl>

#include <t2/state.h>

__kernel void raytracer(
        __constant struct configuration *config,
        __constant struct state *state,
        __read_only image2d_t input,
        __write_only image2d_t output,
        __global float2 *squareSampleSets,
        __global float2 *diskSampleSets,
        int numSampleSets,
        uint batchSize)
{
    __local struct Scene s;

    local int sampleSetSize;

    if ((get_local_id(0) == 0) & (get_local_id(1) == 0)) {
        buildscene(&s);
        sampleSetSize = config->sampleRoot * config->sampleRoot;
    } else {
        sampleSetSize = 0;
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    int sampleSetIndex = ((pos.x * config->height) + pos.y) % numSampleSets;

    // newCVal is where we store the current color.
    float4 newCVal = (float4)(0.f);

    float2 squareSample;
    float2 diskSample;

    // Compute the sample set offset in the sample set buffers based
    // on the coordinates of the current pixel being traced.
    int offset = sampleSetIndex * sampleSetSize;

    // Get pointers to sample sets.
    __global float2 *squareSamples = squareSampleSets + offset;
    __global float2 *diskSamples = diskSampleSets + offset;

    struct PinholeCamera pinhole;
    struct ThinLensCamera thinLens;

    // Configure camera
    if (s.cameraType == CAMERA_THINLENS) {
        thinLens = s.cameras.thinLens;
        thinLens.eye = state->position;
        thinLens.lookat = state->position + state->heading;
        thinLens.lens_radius = state->lens_radius;
        thinlens_camera_compute_uvw(&thinLens);
    } else if (s.cameraType == CAMERA_PINHOLE) {
        pinhole = s.cameras.pinhole;
        pinhole.eye = state->position;
        pinhole.lookat = state->position + state->heading;
        pinhole_camera_compute_uvw(&pinhole);
    }

    for (uint sampleNum = state->sampleNum;
            sampleNum < state->sampleNum + batchSize;
            sampleNum++) {
        // Select samples from sets based on current sample index.
        squareSample = squareSamples[sampleNum];
        diskSample = diskSamples[sampleNum];

        if (s.cameraType == CAMERA_THINLENS) {
            newCVal += thinlens_camera_render(&thinLens, &s,
                    config, pos, squareSample, diskSample);
        } else if (s.cameraType == CAMERA_PINHOLE) {
            newCVal += pinhole_camera_render(&pinhole, &s,
                    config, pos, squareSample);
        }
    }

    // If this isn't the first sample for this frame, combine the new
    // sample with the old ones.
    if (state->sampleNum > 0) {
        // If this isn't the first sample for this frame, read the
        // previous sample data from the input image. Otherwise we take
        // the current sample as the first sample.
        newCVal = (read_imagef(input, pos) * (float)state->sampleNum + newCVal) /
            ((float)state->sampleNum + (float)batchSize);
    } else if (batchSize > 1)
        newCVal /= (float4)(batchSize);

    // Write the final sample value to the output image.
    write_imagef(output, pos, newCVal);
}
