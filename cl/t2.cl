
#include <t2/extensions.cl>
#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/stack.cl>
#include <t2/plane.cl>
#include <t2/sphere.cl>
#include <t2/scene.cl>
#include <t2/trace.cl>
#include <t2/camera.cl>

__kernel void raytracer(__write_only image2d_t output, uint width, uint height,
        float3 position, float3 heading)
{
    struct Scene s;
    buildscene(&s);

    struct Camera camera;

    camera.eye = position;
    camera.lookat = position + heading;
    camera.up = (float3)(0, 1, 0);
    camera.vpdist = 500;
    camera_compute_uvw(&camera);

    float4 cVal = (float4)(0);
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 sample;

    sample = (float2)(pos.x - 0.25, pos.y - 0.25);
    cVal += camera_render(&camera, &s, width, height, sample);
    sample = (float2)(pos.x - 0.25, pos.y + 0.25);
    cVal += camera_render(&camera, &s, width, height, sample);
    sample = (float2)(pos.x + 0.25, pos.y - 0.25);
    cVal += camera_render(&camera, &s, width, height, sample);
    sample = (float2)(pos.x + 0.25, pos.y + 0.25);
    cVal += camera_render(&camera, &s, width, height, sample);

    cVal /= 4.f;

    if (isnan(cVal.x)) {
        cVal = (float4)(1, 0, 0, 1);
    } else if (isnan(cVal.y)) {
        cVal = (float4)(0, 1, 0, 1);
    } else if (isnan(cVal.z)) {
        cVal = (float4)(0, 0, 1, 1);
    }

    write_imagef(output, pos, cVal);
}
