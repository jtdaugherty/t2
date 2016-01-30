
#include <t2/extensions.cl>
#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/scene.cl>
#include <t2/pinhole_camera.cl>
#include <t2/thinlens_camera.cl>

/* This should match the struct in t2/config.h */
struct configuration {
    uint traceDepth;
    int sampleRoot;
    int width;
    int height;
    int _unused_logLevel;
};

__kernel void raytracer(
        __global struct configuration *config,
        __read_only image2d_t input,
        __write_only image2d_t output,
        __global float2 *squareSampleSets,
        __global float2 *diskSampleSets,
        float3 position, float3 heading,
        float lens_radius,
        int numSampleSets,
        uint sampleIdx,
        uint sampleNum)
{
    struct Scene s;
    buildscene(&s);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 origCVal = (float4)(0.f);

    if (sampleNum > 0)
        origCVal = read_imagef(input, pos);

    // Size in float2s
    int sampleSetSize = config->sampleRoot * config->sampleRoot;
    int sampleSetIndex = ((pos.x * config->height) + pos.y) % numSampleSets;
    int offset = sampleSetIndex * sampleSetSize;

    // Select sample sets
    __global float2 *squareSamples = squareSampleSets + offset;
    __global float2 *diskSamples = diskSampleSets + offset;

    // Select samples
    float2 squareSample = squareSamples[sampleIdx];
    float2 diskSample = diskSamples[sampleIdx];

    float4 newCVal;

    // Configure camera and trace ray
    if (s.cameraType == CAMERA_THINLENS) {
        s.cameras.thinLensCamera.eye = position;
        s.cameras.thinLensCamera.lookat = position + heading;
        s.cameras.thinLensCamera.lens_radius = lens_radius;
        thinlens_camera_compute_uvw(&s.cameras.thinLensCamera);

        newCVal = thinlens_camera_render(&s.cameras.thinLensCamera, &s,
                config->width, config->height, config->traceDepth,
                pos, squareSample, diskSample);
    } else if (s.cameraType == CAMERA_PINHOLE) {
        s.cameras.pinholeCamera.eye = position;
        s.cameras.pinholeCamera.lookat = position + heading;
        pinhole_camera_compute_uvw(&s.cameras.pinholeCamera);

        newCVal = pinhole_camera_render(&s.cameras.pinholeCamera, &s,
                config->width, config->height, config->traceDepth,
                pos, squareSample);
    }

    // If this isn't the first sample for this frame, combine the new
    // sample with the old ones.
    if (sampleNum > 0)
        newCVal = (origCVal * sampleNum + newCVal) / (sampleNum + 1);

    // Write the final sample value to the output image.
    write_imagef(output, pos, newCVal);
}
