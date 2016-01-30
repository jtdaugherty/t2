
#ifndef T2_SAMPLERS_H
#define T2_SAMPLERS_H

void mapToUnitDisk(float *x, float *y);
void generateRandomSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*));
void generateJitteredSampleSet(float *samples, int sampleRoot, void(*map)(float*, float*));
void shuffle(int *array, size_t n);

#endif
