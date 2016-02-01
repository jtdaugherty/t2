
#ifndef T2_TYPES_CL
#define T2_TYPES_CL

#define MAX_OBJECTS 20
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
    float3 center;
    float radius;
};

struct Plane
{
    float3 normal;
    float3 origin;
};

enum ObjectType {
    OBJECT_SPHERE,
    OBJECT_PLANE
};

struct Object {
    enum ObjectType type;
    union {
        struct Sphere sphere;
        struct Plane plane;
    } types;
    uint material;
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
    struct Object objects[MAX_OBJECTS];
    struct Light  lights[MAX_LIGHTS];
    struct Material materials[MAX_MATERIALS];

    uint numObjects;
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
