
#ifndef T2_OVERLAY_H
#define T2_OVERLAY_H

#include <t2/state.h>
#include <t2/config.h>

int initialize_overlay(struct configuration *config);
void render_overlay(struct configuration *config, struct state *programState);

#endif
