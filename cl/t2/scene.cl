
#include <t2/types.cl>
#include <t2/thinlens_camera.cl>

static void buildscene(struct Scene *s)
{
    s->cameraType = CAMERA_THINLENS;
    s->cameras.thinLensCamera.up = (float3)(0, 1, 0);
    s->cameras.thinLensCamera.vpdist = 3;
    s->cameras.thinLensCamera.fpdist = 4;

    // s->cameraType = CAMERA_PINHOLE;
    // s->cameras.pinholeCamera.up = (float3)(0, 1, 0);
    // s->cameras.pinholeCamera.vpdist = 3;

    s->numSpheres = 0;

    s->spheres[s->numSpheres].center = (float3)(2, 1, -4);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, -2);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 0);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 2);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 4);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 6);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 8);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(2, 1, 10);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, -4);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, -2);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 0);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 2);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 4);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 6);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 8);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 3;
    s->numSpheres++;

    s->spheres[s->numSpheres].center = (float3)(-2, 1, 10);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres++;

    s->planes[0].normal = (float3)(0, 1, 0);
    s->planes[0].origin = (float3)(0, 0, 0);
    s->planes[0].material = 2;
    s->numPlanes = 1;

    s->materials[0].refl = 0;
    s->materials[0].spec = 127;
    s->materials[0].amb  = (float4)(1, 0.7f, 0.7f, 1);
    s->materials[0].diff = 1;

    s->materials[1].refl = 1;
    s->materials[1].spec = 127;
    s->materials[1].amb  = (float4)(0, 0.7f, 0.7f, 1);
    s->materials[1].diff = 1;

    s->materials[2].refl = 0;
    s->materials[2].spec = 127;
    s->materials[2].amb  = (float4)(1);
    s->materials[2].diff = 1;

    s->materials[3].refl = 1;
    s->materials[3].spec = 127;
    s->materials[3].amb  = (float4)(0.7f, 0, 0.7f, 1);
    s->materials[3].diff = 1;
    s->numMaterials = 4;

    s->lights[0].center = (float3)(-4, 8, 0);
    s->lights[0].strength = 1;
    s->lights[0].color = (float4)(1.0, 243.f/255.f, 168.f/255.f, 1);

    s->numLights = 1;
}
