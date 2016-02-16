
#include <t2/types.cl>

static int planeintersect(__local struct Plane *p, struct Ray *r, float *dist)
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
