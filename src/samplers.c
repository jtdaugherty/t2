
#include <stdlib.h>
#include <math.h>

#include <t2/samplers.h>

static inline float randFloat()
{
    u_int32_t upper_bound = 0xffffffff;
    return ((float)arc4random_uniform(upper_bound))/((float)upper_bound);
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
    for (int i = 0; i < sampleRoot; i++) {
        for (int j = 0; j < sampleRoot; j++) {
            float x = randFloat();
            float y = randFloat();

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }
}

void generateJitteredSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*))
{
    float inc = 1.0 / ((float) sampleRoot);
    for (int i = 0; i < sampleRoot; i++) {
        for (int j = 0; j < sampleRoot; j++) {
            float x = (i * inc) + randFloat() * inc;
            float y = (j * inc) + randFloat() * inc;

            if (map)
                map(&x, &y);

            samples[i * sampleRoot * 2 + j * 2]     = x;
            samples[i * sampleRoot * 2 + j * 2 + 1] = y;
        }
    }
}
