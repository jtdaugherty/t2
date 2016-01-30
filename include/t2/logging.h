
#ifndef T2_LOGGING_H
#define T2_LOGGING_H

#include <stdio.h>

// Log levels
#define LOG_ERROR    (1 << 0)
#define LOG_WARN     (1 << 1)
#define LOG_INFO     (1 << 2)
#define LOG_DEBUG    (1 << 3)

extern int *global_log_level;

#define do_log(level, level_str, fmt, ...) do { \
    if (level <= *global_log_level) { \
        fprintf(stderr, "[" level_str "] "); \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } \
    } while (0)

#define log_error(fmt, ...) do_log(LOG_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  do_log(LOG_INFO,  "INFO",  fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) do_log(LOG_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)

#endif
