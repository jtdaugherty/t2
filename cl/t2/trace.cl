
#define TRACE_DEPTH 10

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

    for (uint i = 0; i < s->numSpheres; i++)
    {
        float lDist = MAXFLOAT;
        int res = sphereintersect(&s->spheres[i], r, &lDist);
        if (res) {
            if (!intersection) {
                return res;
            } else if (lDist < intersection->distance) {
                intersection->result = res;
                intersection->distance = lDist;
                intersection->position = r->origin + r->dir * lDist;
                intersection->normal = spherenormal(&s->spheres[i], intersection->position);
                intersection->material = &s->materials[s->spheres[i].material];
            }
        }
    }

    for (uint i = 0; i < s->numPlanes; i++)
    {
        float lDist = MAXFLOAT;
        int res = planeintersect(&s->planes[i], r, &lDist);
        if (res) {
            if (!intersection) {
                return res;
            } else if (lDist < intersection->distance) {
                intersection->result = res;
                intersection->distance = lDist;
                intersection->position = r->origin + r->dir * lDist;
                intersection->normal = (&s->planes[i])->normal;
                intersection->material = &s->materials[s->planes[i].material];
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

static float4 raytrace(struct Scene *s, struct RayStack *stack, struct Ray *r, float refr, int depth)
{
    float4 color = (float4)(0, 0, 0, 0);

    if(depth > TRACE_DEPTH) return color;

    struct IntersectionResult intersection;
    int result = findintersection(s, r, &intersection);

    if(result == 0) return color;

    struct Material *m  = intersection.material;
    float3 P = intersection.position;
    float3 N = intersection.normal;

    for(uint i = 0; i < s->numLights; i++)
    {
        struct Material *lm = &s->materials[s->lights[i].material];
        float3 L = s->lights[i].center - P;

        float shade = shadowray(s, L, P);

        L = normalize(L);

        float angle = fmax(0.f, dot(N, L)) * shade;
        float s = dot(r->dir, reflect(N, L)) * shade;

        color += angle * m->diff * m->amb * lm->amb 
            + powr(fmax(0.f, s), m->spec) * lm->amb;
    }

    if(m->refl > 0)
    {
        float3 refl = reflect(N, r->dir);

        struct Ray R;
        R.origin = P + refl * EPSILON;
        R.dir = refl;

        push(stack, &R, refr, depth + 1);
    }

    if(m->refr > 0)
    {
        float3 refrN = N * (float)result;
        float n = refr / m->refr;
        float cos_i = -dot(refrN, r->dir);
        float cos_t2 = 1.f - n * n * (1 - cos_i * cos_i);

        if(cos_t2 > 0)
        {
            float3 T = n * r->dir + (n * cos_i - sqrt(cos_t2)) * refrN;

            struct Ray R;
            R.origin = P + T * EPSILON;
            R.dir = T;

            push(stack, &R, m->refr, depth + 1);
        }
    }

    return color;
}

static float4 recursivetrace(struct Scene *s, struct Ray *r)
{
    struct RayStack stack;
    stack.top = 0;
    push(&stack, r, 1.f, 0);

    float4 c = (float4)(0, 0, 0, 0);

    while(stack.top > 0)
    {
        stack.top--;
        c += raytrace(s, &stack, &stack.r[stack.top], stack.refr[stack.top], stack.depth[stack.top]);
    }

    return c;
}
