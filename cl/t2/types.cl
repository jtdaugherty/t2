
#ifndef T2_TYPES_CL
#define T2_TYPES_CL

#define MAX_PLANES 10
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define MAX_MATERIALS 10

struct Ray
{
    float4 origin;
    float4 dir;
};

struct Sphere
{
    uint  material;
    float4 center;
    float radius;
};

struct Plane
{
    uint material;
    float4 normal;
    float4 origin;
};

struct Light
{
    uint  material;
    float4 center;
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
    float4 normal;
    float4 position;
    float distance;
    struct Material *material;
};

#endif
