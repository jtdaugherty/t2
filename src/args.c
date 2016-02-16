
#include <unistd.h>
#include <stdio.h>

#include <t2/config.h>
#include <t2/logging.h>
#include <t2/samplers.h>

void usage(char *progname, struct configuration *config)
{
    printf("Usage: %s [option]\n", progname);
    printf("Options:\n");
    printf("    -h           This help output\n");
    printf("    -d DEPTH     Trace depth (default: %d)\n", config->traceDepth);
    printf("    -r ROOT      Sample root (default: %d, max: %d)\n",
            config->sampleRoot, MAX_SAMPLE_ROOT);
    printf("    -b SIZE      Batch size in samples per kernel invocation (default: %d)\n",
            config->batchSize);
    printf("    -W WIDTH     Scene width (default: %d)\n", config->width);
    printf("    -H HEIGHT    Scene height (default: %d)\n", config->height);
    printf("    -l LEVEL     Log level (default: %s)\n", log_level_name(config->logLevel));
    printf("    -f           Run in windowed fullscreen mode\n");
    exit(1);
}

void processArgs(int argc, char **argv, struct configuration *config)
{
    int ch, logLevel;
    struct configuration newConfig = *config;

    while ((ch = getopt(argc, argv, "b:fhd:r:W:H:l:")) != -1) {
        switch (ch) {
            case 'b':
                if (atoi(optarg) < 0) {
                    goto bad;
                }

                newConfig.batchSize = atoi(optarg);
                break;

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

            case 'f':
                newConfig.fullScreen = 1;
                break;

            case '?':
            case 'h':
bad:
            default:
                usage(argv[0], config);
                return;
        }
    }

    if (optind < argc) {
        usage(argv[0], config);
        return;
    }

    *config = newConfig;
    return;
}
