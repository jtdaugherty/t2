
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

__kernel void raytracer(
        __read_only image2d_t input,
        __write_only image2d_t output,
        uint width, uint height,
        float3 position, float3 heading,
        __global float2 *squareSampleSets,
        __global float2 *diskSampleSets,
        int numSampleSets,
        int sampleRoot,
        uint sampleIdx,
        uint traceDepth)
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
    camera.lens_radius = 0.09;
    thinlens_camera_compute_uvw(&camera);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 origCVal = (float4)(0.f);

    if (sampleIdx > 0) {
        origCVal = read_imagef(input, pos);
    }

    // Size in float2s
    int sampleSetSize = sampleRoot * sampleRoot;
    int sampleSetIndex = ((pos.x * height) + pos.y) % numSampleSets;
    int offset = sampleSetIndex * sampleSetSize;
    __global float2 *squareSamples = squareSampleSets + offset;
    __global float2 *diskSamples = diskSampleSets + offset;

    float2 squareSample = squareSamples[sampleIdx];
    float2 diskSample = diskSamples[sampleIdx];
    // float4 newCVal = pinhole_camera_render(&camera, &s, width, height, traceDepth, pos, squareSample);
    float4 newCVal = thinlens_camera_render(&camera, &s, width, height, traceDepth, pos, squareSample, diskSample);

    if (sampleIdx > 0) {
        newCVal = (origCVal * sampleIdx + newCVal) / (sampleIdx + 1);
    }

    write_imagef(output, pos, newCVal);
}
