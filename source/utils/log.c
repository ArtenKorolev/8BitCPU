#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void emu_log(const log_level_e level, const char *format, ...) {
  if (level > g_log_level) {
    return;
  }

  const char *color = "";
  const char *prefix = "";

  switch (level) {
    case NO_LOG:
      return;
    case ERROR:
      color = "\033[31m";  // red
      prefix = "[ERROR] ";
      break;
    case WARN:
      color = "\033[33m";  // yellow
      prefix = "[WARN] ";
      break;
    case INFO:
      color = "\033[32m";  // green
      prefix = "[INFO] ";
      break;
  }

  printf("%s%s", color, prefix);

  va_list list;
  va_start(list, format);
  vprintf(format, list);
  va_end(list);

  printf("\033[0m");  // reset color
}
