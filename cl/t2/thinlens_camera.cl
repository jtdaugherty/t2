
#ifndef T2_THINLENS_CAMERA_CL
#define T2_THINLENS_CAMERA_CL

#include <t2/types.cl>
#include <t2/trace.cl>
#include <t2/config.cl>

static void thinlens_camera_compute_uvw(struct ThinLensCamera *camera)
{
    camera->w = normalize(camera->eye - camera->lookat);
    camera->u = normalize(cross(camera->up, camera->w));
    camera->v = cross(camera->w, camera->u);
}

static float3 thinlens_camera_ray_direction(struct ThinLensCamera *camera, float2 pixelPoint, float2 lensPoint)
{
    float2 p = (float2)(0);
    p.x = pixelPoint.x * (camera->fpdist / camera->vpdist);
    p.y = pixelPoint.y * (camera->fpdist / camera->vpdist);

    return normalize(((pixelPoint.x - lensPoint.x) * camera->u) +
                     ((pixelPoint.y - lensPoint.y) * camera->v) -
                     (camera->fpdist * camera->w));
}

static float4 thinlens_camera_render(
        struct ThinLensCamera *camera, struct Scene *scene,
        __constant struct configuration *config,
        int2 coord,
        float2 squareSample, float2 diskSample)
{
    float2 pixelPoint = 0.01f * (float2)(coord.x - (config->width / 2.f) + squareSample.x,
                                         coord.y - (config->height / 2.f) + squareSample.y);
    struct Ray r;
    float2 lensPoint = camera->lens_radius * diskSample;

    r.origin = camera->eye + (lensPoint.x * camera->u) + (lensPoint.y * camera->v);
    r.dir = thinlens_camera_ray_direction(camera, pixelPoint, lensPoint);

    return recursivetrace(scene, config->traceDepth, &r);
}

#endif
