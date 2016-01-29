
#include <t2/extensions.cl>
#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/stack.cl>
#include <t2/plane.cl>
#include <t2/sphere.cl>
#include <t2/scene.cl>
#include <t2/trace.cl>
#include <t2/camera.cl>

__kernel void raytracer(
        __read_only image2d_t input,
        __write_only image2d_t output,
        uint width, uint height,
        float3 position, float3 heading,
        __global float2 *samples, uint sampleIdx)
{
    struct Scene s;
    buildscene(&s);

    struct Camera camera;

    camera.eye = position;
    camera.lookat = position + heading;
    camera.up = (float3)(0, 1, 0);
    camera.vpdist = 500;
    camera_compute_uvw(&camera);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 origCVal = (float4)(0.f);

    if (sampleIdx > 0) {
        origCVal = read_imagef(input, pos);
    }

    float2 sample = samples[sampleIdx];
    float2 newPos = (float2)(pos.x + sample.x, pos.y + sample.y);
    float4 newCVal = camera_render(&camera, &s, width, height, newPos);

    if (sampleIdx > 0) {
        newCVal = (origCVal * sampleIdx + newCVal) / (sampleIdx + 1);
    }

    write_imagef(output, pos, newCVal);
}
