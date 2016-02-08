
#ifndef T2_MATHUTIL_H
#define T2_MATHUTIL_H

#define MAXF(a, b) ((a) > (b) ? (a) : (b))
#define MINF(a, b) ((a) < (b) ? (a) : (b))

static inline void normalize(cl_float3 *vec)
{
    cl_float len = sqrt(vec->x * vec->x +
            vec->y * vec->y +
            vec->z * vec->z);

    vec->x /= len;
    vec->y /= len;
    vec->z /= len;
}

#endif
