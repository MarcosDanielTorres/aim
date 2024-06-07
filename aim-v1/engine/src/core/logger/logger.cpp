#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"


void log_output(LOG_LEVEL log_level, const char* msg, const char* file, int line, ...) {
  const char* log_levels[6] = {
    "TRACE",
    "INFO",
    "DEBUG",
    "WARN",
    "ERROR",
    "FATAL",
  };

  const char* color_strings[] = {"38;5;13", "1;32", "1;34", "1;33", "38;5;202", "1;41"};

  const char* log_prefix = log_levels[log_level];

  char buf[3000] = {};
  
  va_list args;
  va_start(args, line);
  int written = vsnprintf(buf, 3000, msg, args);
  buf[written] = 0;
  va_end(args);

  char out_message[8800];
  // Get the file path from the /src folder
  const char* src_path = strstr(file, "/src");
  if (!src_path) {
      src_path = strstr(file, "\\src");
  }
  if (!src_path) {
      src_path = file; // If /src is not found, use the full path
  }

  snprintf(out_message, 8800, "%s %s [%s] %s:%d: %s\n", __DATE__, __TIME__, log_prefix, src_path, line, buf);

 if (log_level > 3){
   fprintf(stderr, "\033[%sm%s\033[0m", color_strings[log_level], out_message);
 }else{
   printf("\033[%sm%s\033[0m", color_strings[log_level], out_message);
 }
}



