
#include <unistd.h>
#include <stdio.h>

#include <t2/config.h>
#include <t2/logging.h>

/* Let's be reasonable. */
#define MAX_SAMPLE_ROOT 32

void usage(char *progname, struct configuration *config)
{
    printf("Usage: %s [option]\n", progname);
    printf("Options:\n");
    printf("    -h           This help output\n");
    printf("    -d DEPTH     Trace depth (default: %d)\n", config->traceDepth);
    printf("    -r ROOT      Sample root (default: %d, max: %d)\n", config->sampleRoot,
                                                                    MAX_SAMPLE_ROOT);
    printf("    -W WIDTH     Window width (default: %d)\n", config->width);
    printf("    -H HEIGHT    Window height (default: %d)\n", config->height);
    printf("    -l LEVEL     Log level (default: %s)\n", log_level_name(config->logLevel));
    exit(1);
}

void processArgs(int argc, char **argv, struct configuration *config)
{
    int ch, logLevel;
    struct configuration newConfig = *config;

    while ((ch = getopt(argc, argv, "hd:r:W:H:l:")) != -1) {
        switch (ch) {
            case 'd':
                if (atoi(optarg) < 0) {
                    goto bad;
                }

                newConfig.traceDepth = atoi(optarg);
                break;

            case 'r':
                if (atoi(optarg) <= 0) {
                    goto bad;
                }

                newConfig.sampleRoot = atoi(optarg);

                if (newConfig.sampleRoot > MAX_SAMPLE_ROOT) {
                    goto bad;
                }
                break;

            case 'W':
                if (atoi(optarg) <= 0) {
                    goto bad;
                }

                newConfig.width = atoi(optarg);
                break;

            case 'H':
                if (atoi(optarg) <= 0) {
                    goto bad;
                }

                newConfig.height = atoi(optarg);
                break;

            case 'l':
                logLevel = log_level_from_name(optarg);
                if (logLevel == -1) {
                    goto bad;
                }

                newConfig.logLevel = logLevel;
                break;

            case '?':
            case 'h':
bad:
            default:
                usage(argv[0], config);
                return;
        }
    }

    *config = newConfig;
    return;
}
