
#ifndef T2_TRACE_CL
#define T2_TRACE_CL

#include <t2/stack.cl>
#include <t2/sphere.cl>
#include <t2/plane.cl>

static float3 reflect(float3 A, float3 B)
{
    return B - ((float3)2) * (float3)dot(A, B) * A;
}

static int findintersection(struct Scene *s, struct Ray *r, struct IntersectionResult *intersection)
{
    if (intersection) {
        intersection->distance = MAXFLOAT;
        intersection->result = 0;
    }

    int hitObject = -1;
    int res;
    float lDist;

    for (uint i = 0; i < s->numObjects; i++)
    {
        lDist = MAXFLOAT;
        if (s->objects[i].type == OBJECT_SPHERE) {
            res = sphereintersect(&s->objects[i].types.sphere, r, &lDist);
        } else if (s->objects[i].type == OBJECT_PLANE) {
            res = planeintersect(&s->objects[i].types.plane, r, &lDist);
        }
        if (res) {
            if (!intersection) {
                // When there's no intersection record to fill out, it's
                // because we're doing a shadow ray trace. Since we
                // don't care which object was closer -- we only care
                // that we hit _something_ - we return as soon as we
                // find any hit at all.
                return res;
            } else if (lDist < intersection->distance) {
                intersection->result = 1;
                intersection->distance = lDist;
                hitObject = i;
            }
        }
    }

    if (intersection) {
        if (intersection->result) {
            intersection->position = r->origin + r->dir * intersection->distance;
            intersection->material = &s->materials[s->objects[hitObject].material];
            if (s->objects[hitObject].type == OBJECT_SPHERE) {
                intersection->normal = spherenormal(&s->objects[hitObject].types.sphere,
                        intersection->position);
            } else if (s->objects[hitObject].type == OBJECT_PLANE) {
                intersection->normal = (&s->objects[hitObject].types.plane)->normal;
            }
        }
        return intersection->result;
    } else
        return 0;
}

static int shadowRayHit(struct Scene *s, float3 L, float3 P)
{
    struct Ray light;
    light.origin = P;
    light.dir = L;

    return findintersection(s, &light, 0);
}

static float4 raytrace(struct Scene *s, struct RayStack *stack, uint traceDepth, struct Ray *r, uint depth)
{
    float4 color = (float4)(0, 0, 0, 0);

    if (depth > traceDepth) return color;

    struct IntersectionResult intersection;
    int result = findintersection(s, r, &intersection);

    if (result == 0) return color;

    struct Material *m  = intersection.material;
    float3 P = intersection.position;
    float3 N = intersection.normal;

    float angle, sv;
    float3 L;
    float4 lColor;
    struct Light *light;

    for (uint i = 0; i < s->numLights; i++)
    {
        light = &(s->lights[i]);
        L = normalize(light->center - P);

        if (shadowRayHit(s, L, P) == 0) {
            angle = fmax(0.f, dot(N, L));
            sv = dot(r->dir, reflect(N, L));

            lColor = (float4)(light->strength) * light->color;
            color += angle * m->diff * m->amb * lColor
                + powr(fmax(0.f, sv), m->spec) * lColor;
        }
    }

    if (m->refl > 0)
    {
        float3 refl = reflect(N, r->dir);

        struct Ray R;
        R.origin = P + refl * EPSILON;
        R.dir = refl;

        push(stack, &R, depth + 1);
    }

    return color;
}

static float4 recursivetrace(struct Scene *s, uint traceDepth, struct Ray *r)
{
    struct RayStack stack;
    stack.top = 0;
    push(&stack, r, 0);

    float4 c = (float4)(0, 0, 0, 0);

    while(stack.top > 0)
    {
        stack.top--;
        c += raytrace(s, &stack, traceDepth, &stack.r[stack.top], stack.depth[stack.top]);
    }

    return c;
}

#endif
