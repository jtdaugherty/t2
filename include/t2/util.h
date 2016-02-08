
#ifndef T2_UTIL_H
#define T2_UTIL_H

#include <sys/time.h>

#include <t2/opencl_setup.h>

cl_program readAndBuildProgram(cl_context context, cl_device_id device_id, const char *path, int *res);
void timevalDiff(struct timeval *start, struct timeval *stop, struct timeval *diff);

#define CLEAR_BIT(mask, bit)  do { mask &= ~bit; } while (0);
#define SET_BIT(mask, bit)    do { mask |= bit; } while (0);

#endif
