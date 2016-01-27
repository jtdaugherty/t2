
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#include <t2/constants.cl>
#include <t2/types.cl>
#include <t2/stack.cl>
#include <t2/plane.cl>
#include <t2/sphere.cl>
#include <t2/scene.cl>
#include <t2/trace.cl>
#include <t2/camera.cl>

__kernel void raytracer(__write_only image2d_t output, uint width, uint height, float z)
{
    struct Scene s;
    s.numSpheres = s.numLights = s.numMaterials = s.numPlanes = 0;
    buildscene(&s);

    struct Camera camera;
    camera.eye = (float3)(0, 20, z);
    camera.lookat = (float3)(0, 1, z);
    camera.up = (float3)(0, 1, 0);
    camera.vpdist = 5;
    camera_compute_uvw(&camera);

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 cVal = camera_render(&camera, &s, width, height, pos);

    if (isnan(cVal.x)) {
        cVal = (float4)(1, 0, 0, 1);
    } else if (isnan(cVal.y)) {
        cVal = (float4)(0, 1, 0, 1);
    } else if (isnan(cVal.z)) {
        cVal = (float4)(0, 0, 1, 1);
    }

    write_imagef(output, pos, cVal);
}
