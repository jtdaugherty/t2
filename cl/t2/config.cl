
#ifndef T2_CONFIG_CL
#define T2_CONFIG_CL

/* This should match the struct in t2/config.h */
struct configuration {
    uint traceDepth;
    int sampleRoot;
    int width;
    int height;
    int _unused_logLevel;
    int _unused_batchSize;
    int _unused_paused;
    int _unused_fullScreen;
};

#endif
