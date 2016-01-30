
#ifndef T2_TYPES_CL
#define T2_TYPES_CL

#define MAX_PLANES 10
#define MAX_SPHERES 20
#define MAX_LIGHTS 10
#define MAX_MATERIALS 10

struct Ray
{
    float3 origin;
    float3 dir;
};

struct PinholeCamera
{
    float3 eye;
    float3 lookat;
    float3 up;
    float vpdist;
    // Computed
    float3 u, v, w;
};

struct ThinLensCamera
{
    float3 eye;
    float3 lookat;
    float3 up;
    float vpdist;
    float fpdist;
    float lens_radius;
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
    float3 center;
    float strength;
    float4 color;
};

struct Material
{
    float refl;
    float diff;
    float spec;
    float4 amb;
};

enum CameraType {
    CAMERA_PINHOLE,
    CAMERA_THINLENS
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

    union {
        struct PinholeCamera pinhole;
        struct ThinLensCamera thinLens;
    } cameras;

    enum CameraType cameraType;
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
