#pragma once
#include <stdio.h> 

enum LOG_LEVEL {
    TRACE = 0,
    INFO = 1,
    DEBUG = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};


void log_output(LOG_LEVEL log_level, const char* message, const char* file, int line, ...);

#define TRACE(msg, ...) log_output(TRACE, msg, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(msg, ...) log_output(INFO, msg, __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(msg, ...) log_output(DEBUG, msg, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(msg, ...) log_output(WARN, msg, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(msg, ...) log_output(ERROR, msg, __FILE__, __LINE__, __VA_ARGS__)
#define FATAL(msg, ...) log_output(FATAL, msg, __FILE__, __LINE__, __VA_ARGS__)
