
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
    buildscene(&s);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));

    // newCVal is where we store the current color.
    float4 newCVal = (float4)(0.f);

    float2 squareSample;
    float2 diskSample;

    int sampleSetSize = config->sampleRoot * config->sampleRoot;
    int sampleSetIndex = ((pos.x * config->height) + pos.y) % numSampleSets;

    // Compute the sample set offset in the sample set buffers based
    // on the coordinates of the current pixel being traced.
    int offset = sampleSetIndex * sampleSetSize;

    // Get pointers to sample sets.
    __global float2 *squareSamples = squareSampleSets + offset;
    __global float2 *diskSamples = diskSampleSets + offset;

    // Configure camera
    if (s.cameraType == CAMERA_THINLENS) {
        s.cameras.thinLens.eye = state->position;
        s.cameras.thinLens.lookat = state->position + state->heading;
        s.cameras.thinLens.lens_radius = state->lens_radius;
        thinlens_camera_compute_uvw(&s.cameras.thinLens);
    } else if (s.cameraType == CAMERA_PINHOLE) {
        s.cameras.pinhole.eye = state->position;
        s.cameras.pinhole.lookat = state->position + state->heading;
        pinhole_camera_compute_uvw(&s.cameras.pinhole);
    }

    for (uint sampleNum = state->sampleNum;
            sampleNum < state->sampleNum + batchSize;
            sampleNum++) {
        // Select samples from sets based on current sample index.
        squareSample = squareSamples[sampleNum];
        diskSample = diskSamples[sampleNum];

        if (s.cameraType == CAMERA_THINLENS) {
            newCVal += thinlens_camera_render(&s.cameras.thinLens, &s,
                    config, pos, squareSample, diskSample);
        } else if (s.cameraType == CAMERA_PINHOLE) {
            newCVal += pinhole_camera_render(&s.cameras.pinhole, &s,
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
