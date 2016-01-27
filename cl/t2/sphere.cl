
#include <t2/types.cl>

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

