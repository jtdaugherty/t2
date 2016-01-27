#define STACK_DEPTH 20
#define TRACE_DEPTH 10

#define MAX_PLANES 10
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define MAX_MATERIALS 10

#define EPSILON 0.001f

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

struct Ray
{
    float4 origin;
    float4 dir;
};

struct Sphere
{
    uint  material;
    float4 center;
    float sqradius;
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

static void buildscene(struct Scene *s)
{
    s->spheres[s->numSpheres].center = (float4)(0, 1, 0, 0);
    s->spheres[s->numSpheres].sqradius = 1; // 2.5f * 2.5f;
    s->spheres[s->numSpheres].material = 0;
    s->numSpheres = 1;

    s->planes[s->numPlanes].normal = (float4)(0, 1, 0, 0);
    s->planes[s->numPlanes].origin = (float4)(0, 0, 0, 0);
    s->planes[s->numPlanes].material = 0;
    s->numPlanes = 1;

    s->materials[0].refl = 0;
    s->materials[0].refr = 0;
    s->materials[0].spec = 127;
    s->materials[0].amb  = (float4)(0.7f, 0.7f, 0.7f, 0);
    s->materials[0].diff = 1;
    s->numMaterials = 1;

    s->lights[0].center = (float4)(0, 5, 0, 0);
    s->lights[0].material = 0;
    s->numLights = 1;
}

static int planeintersect(struct Plane *p, struct Ray *r, float *dist)
{
    float denom = dot(r->dir, p->normal);

    if (denom == 0.0) {
        return 0;
    }

    float t = dot(p->origin - r->origin, p->normal) / denom;

    if (t > EPSILON) {
        *dist = t;
        return 1;
    } else {
        return 0;
    }
}

static float4 spherenormal(struct Sphere *s, float4 poi)
{
    return normalize(poi - s->center);
}

static int sphereintersect(struct Sphere *s, struct Ray *r, float *dist)
{
    float4 v = r->origin - s->center;
    float b = -dot(v, r->dir);
    float det = (b * b) - dot(v, v) + s->sqradius;

    if(det > 0)
    {
        det = sqrt(det);
        float i1 = b - det;
        float i2 = b + det;

        if(i2 > 0)
        {
            if(i1 < 0)
            {
                if(i2 < *dist) { *dist = i2; return -1; }
            }
            else
            {
                if(i1 < *dist) { *dist = i1; return 1; }
            }
        }
    }

    return 0;
}

static float4 reflect(float4 A, float4 B)
{
    return B - ((float4)2) * (float4)dot(A, B) * A;
}

struct RayStack
{
    struct Ray r[STACK_DEPTH];
    int depth[STACK_DEPTH];
    float refr[STACK_DEPTH];
    int top;
};

static void push(struct RayStack *s, struct Ray *r, float refr, int depth)
{
    if(s->top < STACK_DEPTH)
    {
        s->r[s->top].dir = r->dir;
        s->r[s->top].origin = r->origin;
        s->refr[s->top] = refr;
        s->depth[s->top] = depth;
        s->top++;
    }
}

struct IntersectionResult
{
    int result;
    float4 normal;
    float4 position;
    float distance;
    struct Material *material;
};

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

static float shadowray(struct Scene *s, float4 L, float4 P)
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
    float4 P = intersection.position;
    float4 N = intersection.normal;

    for(uint i = 0; i < s->numLights; i++)
    {
        struct Material *lm = &s->materials[s->lights[i].material];
        float4 L = s->lights[i].center - P;

        float shade = shadowray(s, L, P);

        L = normalize(L);

        float angle = fmax(0.f, dot(N, L)) * shade;
        float s = dot(r->dir, reflect(N, L)) * shade;

        color += angle * m->diff * m->amb * lm->amb 
            + powr(fmax(0.f, s), m->spec) * lm->amb;
    }

    if(m->refl > 0)
    {
        float4 refl = reflect(N, r->dir);

        struct Ray R;
        R.origin = P + refl * EPSILON;
        R.dir = refl;

        push(stack, &R, refr, depth + 1);
    }

    if(m->refr > 0)
    {
        float4 refrN = N * (float)result;
        float n = refr / m->refr;
        float cos_i = -dot(refrN, r->dir);
        float cos_t2 = 1.f - n * n * (1 - cos_i * cos_i);

        if(cos_t2 > 0)
        {
            float4 T = n * r->dir + (n * cos_i - sqrt(cos_t2)) * refrN;

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

__kernel void raytracer(__write_only image2d_t output, uint width, uint height, float z)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 screen = (float2)(
            pos.x / (float)width * 8.f - 4,
            pos.y / (float)height * 6.f - 3
            );

    struct Ray r;

    r.origin = (float4)(0, 1, z, 0);
    r.dir    = normalize((float4)(screen.x, screen.y, 0, 0) - r.origin);

    struct Scene s;
    s.numSpheres = s.numLights = s.numMaterials = s.numPlanes = 0;
    buildscene(&s);

    float4 cVal = recursivetrace(&s, &r);

    if (isnan(cVal.x)) {
        write_imagef(output, pos, (float4)(1, 0, 0, 1));
    } else if (isnan(cVal.y)) {
        write_imagef(output, pos, (float4)(0, 1, 0, 1));
    } else if (isnan(cVal.z)) {
        write_imagef(output, pos, (float4)(0, 0, 1, 1));
    } else {
        write_imagef(output, pos, cVal);
    }
}
