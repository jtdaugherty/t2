
#include <t2/types.cl>

static void buildscene(struct Scene *s)
{
    s->spheres[0].center = (float3)(1, 1, 0);
    s->spheres[0].radius = 1;
    s->spheres[0].material = 0;

    s->spheres[1].center = (float3)(-1, 1, 0);
    s->spheres[1].radius = 1;
    s->spheres[1].material = 1;

    s->spheres[2].center = (float3)(0, 1, -sqrt(6.f));
    s->spheres[2].radius = 1;
    s->spheres[2].material = 3;
    s->numSpheres = 3;

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
    s->lights[0].strength = 0.7;
    s->lights[0].color = (float4)(1, 0.7, 0.7, 1);

    s->lights[1].center = (float3)(4, 6, 0);
    s->lights[1].strength = 0.2;
    s->lights[1].color = (float4)(0.7, 0.7, 1, 1);
    s->numLights = 2;
}
