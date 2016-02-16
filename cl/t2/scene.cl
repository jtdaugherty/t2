
#include <t2/types.cl>
#include <t2/thinlens_camera.cl>

static void buildscene(struct Scene *s)
{
    s->cameraType = CAMERA_THINLENS;
    s->cameras.thinLens.up = (float3)(0, 1, 0);
    s->cameras.thinLens.vpdist = 3;
    s->cameras.thinLens.fpdist = 4;

    // s->cameraType = CAMERA_PINHOLE;
    // s->cameras.pinhole.up = (float3)(0, 1, 0);
    // s->cameras.pinhole.vpdist = 3;

    s->numObjects = 0;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, -4);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, -2);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 1;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 0);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 4;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 2);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 1;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 4);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 4;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 6);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 8);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(2, 1, 10);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 1;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, -4);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, -2);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 5;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 0);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 2);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 5;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 4);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 4;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 6);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 1;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 8);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 3;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_SPHERE;
    s->objects[s->numObjects].types.sphere.center = (float3)(-2, 1, 10);
    s->objects[s->numObjects].types.sphere.radius = 1;
    s->objects[s->numObjects].material = 5;
    s->numObjects++;

    s->objects[s->numObjects].type = OBJECT_PLANE;
    s->objects[s->numObjects].types.plane.normal = (float3)(0, 1, 0);
    s->objects[s->numObjects].types.plane.origin = (float3)(0, 0, 0);
    s->objects[s->numObjects].material = 2;
    s->numObjects++;

    s->materials[0].refl = 0;
    s->materials[0].reflAmount = 1;
    s->materials[0].spec = 127;
    s->materials[0].specAmount = 1;
    s->materials[0].amb  = (float4)(1, 0.7f, 0.7f, 1);
    s->materials[0].diff = 1;

    s->materials[1].refl = 1;
    s->materials[1].reflAmount = 1;
    s->materials[1].spec = 127;
    s->materials[1].specAmount = 1;
    s->materials[1].amb  = (float4)(0, 0.7f, 0.7f, 1);
    s->materials[1].diff = 1;

    s->materials[2].refl = 0;
    s->materials[2].reflAmount = 1;
    s->materials[2].spec = 1;
    s->materials[2].specAmount = 0;
    s->materials[2].amb  = (float4)(1);
    s->materials[2].diff = 1;

    s->materials[3].refl = 1;
    s->materials[3].reflAmount = 0.1;
    s->materials[3].spec = 10;
    s->materials[3].specAmount = 1;
    s->materials[3].amb  = (float4)(0.7f, 0, 0.7f, 1);
    s->materials[3].diff = 1;
    s->numMaterials = 4;

    s->materials[4].refl = 0;
    s->materials[4].reflAmount = 1;
    s->materials[4].spec = 64;
    s->materials[4].specAmount = 0.5;
    s->materials[4].amb  = (float4)(1.f, 0, 0, 1);
    s->materials[4].diff = 1;

    s->materials[5].refl = 1;
    s->materials[5].reflAmount = 0;
    s->materials[5].spec = 2000;
    s->materials[5].specAmount = 1;
    s->materials[5].amb  = (float4)(0.3f, 0.3f, 1.f, 1.f);
    s->materials[5].diff = 1;
    s->numMaterials = 5;

    s->lights[0].center = (float3)(0, 30, 0);
    s->lights[0].strength = 0.9;
    s->lights[0].color = (float4)(1.0, 243.f/255.f, 168.f/255.f, 1);

    s->numLights = 1;
}
