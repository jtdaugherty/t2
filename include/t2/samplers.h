
#ifndef T2_SAMPLERS_H
#define T2_SAMPLERS_H

/* Let's be reasonable. */
#define MAX_SAMPLE_ROOT 32

void mapToUnitDisk(float *x, float *y);
void generateRandomSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*));
void generateJitteredSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*));
void shuffle(void *buf, size_t n, size_t elem_size);

#endif
