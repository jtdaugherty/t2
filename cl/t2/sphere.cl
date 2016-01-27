
#include <t2/types.cl>

static float3 spherenormal(struct Sphere *s, float3 poi)
{
    return normalize(poi - s->center);
}

static int sphereintersect(struct Sphere *s, struct Ray *r, float *dist)
{
    float3 temp = r->origin - s->center;
    float a = dot(r->dir, r->dir);
    float b = dot((float3)(2.0) * temp, r->dir);
    float c = dot(temp, temp) - (s->radius * s->radius);
    float disc = b * b - (4.0 * a * c);

    if (disc < 0) {
        return 0;
    } else {
        float e = sqrt(disc);
        float denom = 2.0 * a;
        float t = (-b - e) / denom;

        if (t > EPSILON) {
            *dist = t;
            return 1;
        }

        t = (-b + e) / denom;

        if (t > EPSILON) {
            *dist = t;
            return 1;
        }
    }

    return 0;
}
