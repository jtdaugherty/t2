
#ifndef T2_STACK_CL
#define T2_STACK_CL

#include <t2/types.cl>

#define STACK_DEPTH 20

struct RayStack
{
    struct Ray r[STACK_DEPTH];
    int depth[STACK_DEPTH];
    float contribAmount[STACK_DEPTH];
    int top;
};

static void push(struct RayStack *s, struct Ray *r, int depth, float contribAmount)
{
    if (s->top < STACK_DEPTH) {
        s->r[s->top].dir = r->dir;
        s->r[s->top].origin = r->origin;
        s->depth[s->top] = depth;
        s->contribAmount[s->top] = contribAmount;
        s->top++;
    }
}

#endif
