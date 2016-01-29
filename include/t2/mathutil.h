
#ifndef T2_MATHUTIL_H
#define T2_MATHUTIL_H

#define MAXF(a, b) ((a) > (b) ? (a) : (b))

static inline void normalize(cl_float *vec)
{
    cl_float len = sqrt(vec[0] * vec[0] +
            vec[1] * vec[1] +
            vec[2] * vec[2]);

    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

#endif
