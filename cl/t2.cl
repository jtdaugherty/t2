
#include <t2/extensions.cl>
#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/stack.cl>
#include <t2/plane.cl>
#include <t2/sphere.cl>
#include <t2/scene.cl>
#include <t2/trace.cl>
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

    // struct PinholeCamera camera;
    // camera.eye = position;
    // camera.lookat = position + heading;
    // camera.up = (float3)(0, 1, 0);
    // camera.vpdist = 500;
    // pinhole_camera_compute_uvw(&camera);

    struct ThinLensCamera camera;
    camera.eye = position;
    camera.lookat = position + heading;
    camera.up = (float3)(0, 1, 0);
    camera.vpdist = 3;
    camera.fpdist = 4;
    camera.lens_radius = lens_radius;
    thinlens_camera_compute_uvw(&camera);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 origCVal = (float4)(0.f);

    if (sampleNum > 0) {
        origCVal = read_imagef(input, pos);
    }

    // Size in float2s
    int sampleSetSize = config->sampleRoot * config->sampleRoot;
    int sampleSetIndex = ((pos.x * config->height) + pos.y) % numSampleSets;
    int offset = sampleSetIndex * sampleSetSize;
    __global float2 *squareSamples = squareSampleSets + offset;
    __global float2 *diskSamples = diskSampleSets + offset;

    float2 squareSample = squareSamples[sampleIdx];
    float2 diskSample = diskSamples[sampleIdx];
    // float4 newCVal = pinhole_camera_render(&camera, &s, width, height, traceDepth, pos, squareSample);
    float4 newCVal = thinlens_camera_render(&camera, &s, config->width, config->height, config->traceDepth, pos, squareSample, diskSample);

    if (sampleNum > 0) {
        newCVal = (origCVal * sampleNum + newCVal) / (sampleNum + 1);
    }

    write_imagef(output, pos, newCVal);
}
