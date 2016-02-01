
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

    for (uint i = 0; i < s->numObjects; i++)
    {
        float lDist = MAXFLOAT;
        int res;
        if (s->objects[i].type == OBJECT_SPHERE) {
            res = sphereintersect(&s->objects[i].types.sphere, r, &lDist);
        } else if (s->objects[i].type == OBJECT_PLANE) {
            res = planeintersect(&s->objects[i].types.plane, r, &lDist);
        }
        if (res) {
            if (!intersection) {
                return res;
            } else if (lDist < intersection->distance) {
                intersection->result = res;
                intersection->distance = lDist;
                intersection->position = r->origin + r->dir * lDist;

                if (s->objects[i].type == OBJECT_SPHERE) {
                    intersection->normal = spherenormal(&s->objects[i].types.sphere, intersection->position);
                    intersection->material = &s->materials[s->objects[i].material];
                } else if (s->objects[i].type == OBJECT_PLANE) {
                    intersection->normal = (&s->objects[i].types.plane)->normal;
                    intersection->material = &s->materials[s->objects[i].material];
                }
            }
        }
    }

    if (intersection)
        return intersection->result;
    else
        return 0;
}

static float shadowray(struct Scene *s, float3 L, float3 P)
{
    float t = length(L);
    L *= 1.f / t;

    struct Ray light;
    light.origin = P + L * EPSILON;
    light.dir = L;

    return findintersection(s, &light, 0) ? 0.f : 1.f;
}

static float4 raytrace(struct Scene *s, struct RayStack *stack, uint traceDepth, struct Ray *r, uint depth)
{
    float4 color = (float4)(0, 0, 0, 0);

    if(depth > traceDepth) return color;

    struct IntersectionResult intersection;
    int result = findintersection(s, r, &intersection);

    if(result == 0) return color;

    struct Material *m  = intersection.material;
    float3 P = intersection.position;
    float3 N = intersection.normal;

    for(uint i = 0; i < s->numLights; i++)
    {
        float lStrength = s->lights[i].strength;
        float4 lColor = s->lights[i].color;
        float3 L = s->lights[i].center - P;

        float shade = shadowray(s, L, P);

        L = normalize(L);

        float angle = fmax(0.f, dot(N, L)) * shade;
        float s = dot(r->dir, reflect(N, L)) * shade;

        color += angle * m->diff * m->amb * float4(lStrength) * lColor
            + powr(fmax(0.f, s), m->spec) * float4(lStrength) * lColor;
    }

    if(m->refl > 0)
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
