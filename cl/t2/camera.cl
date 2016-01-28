
#include <t2/types.cl>

static void camera_compute_uvw(struct Camera *camera)
{
    camera->w = normalize(camera->eye - camera->lookat);
    camera->u = normalize(cross(camera->up, camera->w));
    camera->v = cross(camera->w, camera->u);
}

static float3 camera_ray_direction(struct Camera *camera, float2 point)
{
    return normalize((point.x * camera->u) +
                     (point.y * camera->v) -
                     (camera->vpdist * camera->w));
}

static float4 camera_render(struct Camera *camera, struct Scene *scene, int width, int height, float2 coord)
{
    float2 pp = (float2)(coord.x - (width / 2.f),
                         coord.y - (height / 2.f));
    struct Ray r;

    r.origin = camera->eye;
    r.dir = camera_ray_direction(camera, pp);

    return recursivetrace(scene, &r);
}
