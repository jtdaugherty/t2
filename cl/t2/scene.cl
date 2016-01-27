
#include <t2/types.cl>

static void buildscene(struct Scene *s)
{
    s->spheres[s->numSpheres].center = (float3)(1, 1, 0);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 0;
    s->numSpheres = 1;

    s->spheres[s->numSpheres].center = (float3)(-1, 1, 0);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 1;
    s->numSpheres = 2;

    s->planes[s->numPlanes].normal = (float3)(0, 1, 0);
    s->planes[s->numPlanes].origin = (float3)(0, 0, 0);
    s->planes[s->numPlanes].material = 1;
    s->numPlanes = 1;

    s->materials[0].refl = 0;
    s->materials[0].refr = 0;
    s->materials[0].spec = 127;
    s->materials[0].amb  = (float4)(1, 0.7f, 0.7f, 1);
    s->materials[0].diff = 1;

    s->materials[1].refl = 1;
    s->materials[1].refr = 0;
    s->materials[1].spec = 127;
    s->materials[1].amb  = (float4)(0, 0.7f, 0.7f, 1);
    s->materials[1].diff = 1;
    s->numMaterials = 2;

    s->lights[0].center = (float3)(4, 5, 0);
    s->lights[0].strength = 0.8;
    s->lights[0].color = (float4)(1, 0, 0, 1);

    s->lights[1].center = (float3)(-4, 2, 0);
    s->lights[1].strength = 0.9;
    s->lights[1].color = (float4)(0.7, 0.7, 1, 1);
    s->numLights = 2;
}
