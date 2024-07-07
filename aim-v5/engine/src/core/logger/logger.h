#pragma once
#include <stdio.h> 
#include "../../defines.h"

enum LOG_LEVEL {
    TRACE = 0,
    INFO = 1,
    DEBUG = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};


void AIM_API aim_log_output(LOG_LEVEL log_level, const char* message, const char* file, int line, ...);

#define TRACE(msg, ...) aim_log_output(TRACE, msg, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(msg, ...) aim_log_output(INFO, msg, __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(msg, ...) aim_log_output(DEBUG, msg, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(msg, ...) aim_log_output(WARN, msg, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(msg, ...) aim_log_output(ERROR, msg, __FILE__, __LINE__, __VA_ARGS__)
#define FATAL(msg, ...) aim_log_output(FATAL, msg, __FILE__, __LINE__, __VA_ARGS__)
