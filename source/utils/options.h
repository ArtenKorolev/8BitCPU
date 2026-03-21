#pragma once

#include <stdbool.h>

#include "base.h"
#include "log.h"

typedef struct Options {
  log_level_e log_level;
  word_t origin;
  const char *rom_file;

  const char *error_msg;
  bool empty;
} options_t;
