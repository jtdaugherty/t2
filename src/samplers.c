
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <t2/samplers.h>

static inline float randFloat()
{
    u_int32_t upper_bound = 0xffffffff;
    return ((float)arc4random_uniform(upper_bound))/((float)upper_bound);
}

void shuffle(void *buf, size_t n, size_t elem_size)
{
    void *item;

    if (n > 1) {
        item = malloc(elem_size);

        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (arc4random_uniform(i+1));

            memcpy(item, buf + (j * elem_size), elem_size);
            memcpy(buf + (j * elem_size), buf + (i * elem_size), elem_size);
            memcpy(buf + (i * elem_size), item, elem_size);
        }

        free(item);
    }
}

void mapToUnitDisk(float *x, float *y)
{
    float spX, spY, phi, r;

    spX = 2.0 * (*x) - 1.0;
    spY = 2.0 * (*y) - 1.0;

    if (spX > -spY) {
        if (spX > spY) {
            r = spX;
            phi = spY / spX;
        } else {
            r = spY;
            phi = 2.0 - spX / spY;
        }
    } else {
        if (spX < spY) {
            r = -spX;
            phi = 4 + spY / spX;
        } else {
            r = -spY;
            if (spY != 0.0) {
                phi = 6.0 - spX / spY;
            } else {
                phi = 0.0;
            }
        }
    }

    phi *= M_PI / 4.0;
    *x = r * cos(phi);
    *y = r * sin(phi);
}

void generateRandomSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*))
{
    float x, y;
    int i, j;

    for (i = 0; i < sampleRoot; i++) {
        for (j = 0; j < sampleRoot; j++) {
            x = randFloat();
            y = randFloat();

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }

    if (sampleRoot > 1)
        shuffle(samples, sampleRoot * sampleRoot, sizeof(float) * 2);
}

void generateJitteredSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*))
{
    float inc = 1.0 / ((float) sampleRoot);
    float x, y;
    int i, j;

    for (i = 0; i < sampleRoot; i++) {
        for (j = 0; j < sampleRoot; j++) {
            x = (i * inc) + randFloat() * inc;
            y = (j * inc) + randFloat() * inc;

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }

    if (sampleRoot > 1)
        shuffle(samples, sampleRoot * sampleRoot, sizeof(float) * 2);
}
