
#ifndef T2_TYPES_CL
#define T2_TYPES_CL

#define MAX_PLANES 10
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define MAX_MATERIALS 10

struct Ray
{
    float3 origin;
    float3 dir;
};

struct Camera
{
    float3 eye;
    float3 lookat;
    float3 up;
    float vpdist;
    // Computed
    float3 u, v, w;
};

struct Sphere
{
    uint  material;
    float3 center;
    float radius;
};

struct Plane
{
    uint material;
    float3 normal;
    float3 origin;
};

struct Light
{
    uint  material;
    float3 center;
};

struct Material
{
    float refl;
    float refr;
    float diff;
    float spec;
    float4 amb;
};

struct Scene
{
    struct Plane planes[MAX_PLANES];
    struct Sphere spheres[MAX_SPHERES];
    struct Light  lights[MAX_LIGHTS];
    struct Material materials[MAX_MATERIALS];

    uint numPlanes;
    uint numSpheres;
    uint numLights;
    uint numMaterials;
};

struct IntersectionResult
{
    int result;
    float3 normal;
    float3 position;
    float distance;
    struct Material *material;
};

#endif
