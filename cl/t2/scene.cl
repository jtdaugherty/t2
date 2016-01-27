
#include <t2/types.cl>

static void buildscene(struct Scene *s)
{
    s->spheres[s->numSpheres].center = (float4)(0, 1, 0, 0);
    s->spheres[s->numSpheres].radius = 1;
    s->spheres[s->numSpheres].material = 0;
    s->numSpheres = 1;

    s->planes[s->numPlanes].normal = (float4)(0, 1, 0, 0);
    s->planes[s->numPlanes].origin = (float4)(0, 0, 0, 0);
    s->planes[s->numPlanes].material = 1;
    s->numPlanes = 1;

    s->materials[0].refl = 0;
    s->materials[0].refr = 0;
    s->materials[0].spec = 127;
    s->materials[0].amb  = (float4)(1, 0.7f, 0.7f, 0);
    s->materials[0].diff = 1;

    s->materials[1].refl = 1;
    s->materials[1].refr = 0;
    s->materials[1].spec = 127;
    s->materials[1].amb  = (float4)(0, 0.7f, 0.7f, 0);
    s->materials[1].diff = 1;
    s->numMaterials = 2;

    s->lights[0].center = (float4)(4, 5, 0, 0);
    s->lights[0].material = 0;
    s->numLights = 1;
}
