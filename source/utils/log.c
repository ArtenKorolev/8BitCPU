#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void emu_log(const log_level_e level, const char *format, ...) {
  if (level > g_log_level) {
    return;
  }

  switch (level) {
    case NO_LOG:
      return;
    case ERROR:
      printf("[ERROR] ");
      break;
    case WARN:
      printf("[WARN] ");
      break;
    case INFO:
      printf("[INFO] ");
      break;
  }

  va_list list;
  va_start(list, format);
  vprintf(format, list);
  va_end(list);
}
