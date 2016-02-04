
#include <t2/overlay.h>
#include <t2/state.h>
#include <t2/config.h>
#include <t2/logging.h>
#include <t2/text.h>

#define OVERLAY_FONT_FILENAME "fonts/InputMono-Regular.ttf"
#define OVERLAY_FONT_PIXEL_HEIGHT 18

static struct text_configuration *text_config = NULL;
static struct font stats_font;

static float overlay_text_color[3] = { 1, 1, 1 };

int initialize_overlay(struct configuration *config)
{
    int ret;

    text_config = initializeText(config);

    ret = loadFont(OVERLAY_FONT_FILENAME, &stats_font, OVERLAY_FONT_PIXEL_HEIGHT);
    if (ret) {
        log_error("Could not load overlay font %s, exiting", OVERLAY_FONT_FILENAME);
        return 1;
    } else
        log_info("Loaded overlay font %s", OVERLAY_FONT_FILENAME);

    return 0;
}

void render_overlay(struct configuration *config, struct state *programState)
{
    char msg[128];
    int len;
    int rowSpacing = 4;
    int rowHeight = stats_font.pixel_height + rowSpacing;
    int bottom = 7;
    int left = 5;
#define ROWS(n) (n * rowHeight)

    len = snprintf(msg, sizeof(msg), "%d/%d sample%s | radius %f | depth %d",
            programState->sampleIdx, config->sampleRoot * config->sampleRoot,
            (config->sampleRoot == 1 ? "" : "s"),
            programState->lens_radius,
            config->traceDepth);

    renderText(text_config, &stats_font, msg, len, left, bottom, 1, overlay_text_color);

    char frameTimeMsg[64];
    if (programState->last_frame_time != -1) {
        len = snprintf(frameTimeMsg, sizeof(frameTimeMsg), "Frame time: %f sec",
                programState->last_frame_time);
        renderText(text_config, &stats_font, frameTimeMsg, len, left, bottom + ROWS(1),
                1, overlay_text_color);
    } else {
        len = snprintf(frameTimeMsg, sizeof(frameTimeMsg), "Frame time: ...");
        renderText(text_config, &stats_font, frameTimeMsg, len, left, bottom + ROWS(1),
                1, overlay_text_color);
    }
}
