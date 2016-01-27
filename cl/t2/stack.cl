
#include <t2/types.cl>

#define STACK_DEPTH 20

struct RayStack
{
    struct Ray r[STACK_DEPTH];
    int depth[STACK_DEPTH];
    float refr[STACK_DEPTH];
    int top;
};

static void push(struct RayStack *s, struct Ray *r, float refr, int depth)
{
    if(s->top < STACK_DEPTH)
    {
        s->r[s->top].dir = r->dir;
        s->r[s->top].origin = r->origin;
        s->refr[s->top] = refr;
        s->depth[s->top] = depth;
        s->top++;
    }
}
