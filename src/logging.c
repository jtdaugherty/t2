
#include <stdio.h>
#include <string.h>

#include <t2/logging.h>

const char * log_level_name(level)
{
    switch (level) {
        case LOG_ERROR:
            return "ERROR";
        case LOG_WARN:
            return "WARN";
        case LOG_INFO:
            return "INFO";
        case LOG_DEBUG:
            return "DEBUG";
    }

    return NULL;
}

int log_level_from_name(char *name)
{
    if (strcmp(name, "ERROR") == 0) {
        return LOG_ERROR;
    } else if (strcmp(name, "WARN") == 0) {
        return LOG_WARN;
    } else if (strcmp(name, "INFO") == 0) {
        return LOG_INFO;
    } else if (strcmp(name, "DEBUG") == 0) {
        return LOG_DEBUG;
    } else {
        return -1;
    }
}
