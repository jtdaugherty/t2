
#include <t2/types.cl>

static void pinhole_camera_compute_uvw(struct PinholeCamera *camera)
{
    camera->w = normalize(camera->eye - camera->lookat);
    camera->u = normalize(cross(camera->up, camera->w));
    camera->v = cross(camera->w, camera->u);
}

static float3 pinhole_camera_ray_direction(struct PinholeCamera *camera, float2 point)
{
    return normalize((point.x * camera->u) +
                     (point.y * camera->v) -
                     (camera->vpdist * camera->w));
}

static float4 pinhole_camera_render(struct PinholeCamera *camera, struct Scene *scene, int width, int height, uint traceDepth, float2 coord)
{
    float2 pp = (float2)(coord.x - (width / 2.f),
                         coord.y - (height / 2.f));
    struct Ray r;

    r.origin = camera->eye;
    r.dir = pinhole_camera_ray_direction(camera, pp);

    return recursivetrace(scene, traceDepth, &r);
}
