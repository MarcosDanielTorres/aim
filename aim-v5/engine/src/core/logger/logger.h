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

#define AIM_TRACE(msg, ...) aim_log_output(LOG_LEVEL::TRACE, msg, __FILE__, __LINE__, __VA_ARGS__)
#define AIM_INFO(msg, ...) aim_log_output(LOG_LEVEL::INFO, msg, __FILE__, __LINE__, __VA_ARGS__)
#define AIM_DEBUG(msg, ...) aim_log_output(LOG_LEVEL::DEBUG, msg, __FILE__, __LINE__, __VA_ARGS__)
#define AIM_WARN(msg, ...) aim_log_output(LOG_LEVEL::WARN, msg, __FILE__, __LINE__, __VA_ARGS__)
#define AIM_ERROR(msg, ...) aim_log_output(LOG_LEVEL::ERROR, msg, __FILE__, __LINE__, __VA_ARGS__)
#define AIM_FATAL(msg, ...) aim_log_output(LOG_LEVEL::FATAL, msg, __FILE__, __LINE__, __VA_ARGS__)
