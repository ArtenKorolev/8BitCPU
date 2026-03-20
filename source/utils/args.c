#include "args.h"

#include <stdlib.h>
#include <string.h>

#include "options.h"

#define LOG_FLAG "-L"
#define ORIGIN_FLAG "-O"

#define LOG_INFO "info"
#define LOG_WARN "warn"
#define LOG_ERR "err"

#include "log.h"

void parse_log_level(const int log_level_index, const char **argv, options_t *options) {
  if (strncmp(argv[log_level_index], LOG_INFO, 4) == 0) {
    options->log_level = INFO;
  } else if (strncmp(argv[log_level_index], LOG_WARN, 4) == 0) {
    options->log_level = WARN;
  } else if (strncmp(argv[log_level_index], LOG_ERR, 3) == 0) {
    options->log_level = ERROR;
  }
}

void parse_origin(const int origin_index, const char **argv, options_t *options) {
  char *endptr = NULL;

  const int origin = (int)strtol(argv[origin_index], &endptr, 16);

  if (*endptr != '\0') {
    options->error_msg = "Unable to parse origin";
    return;
  }

  options->origin = origin;
}

options_t parse_args(const int argc, const char **argv) {
  options_t options = {0};

  for (int i = 1; i < argc; ++i) {
    if (*argv[i] == '-' && i == argc - 1) {
      options.error_msg = "Flag without value";
      break;
    }

    if (strncmp(argv[i], LOG_FLAG, 2) == 0) {
      parse_log_level(++i, argv, &options);
    } else if (strncmp(argv[i], ORIGIN_FLAG, 2) == 0) {
      parse_origin(++i, argv, &options);
    } else {
      if (options.rom_file != NULL) {
        options.error_msg = "The ROM file has been already specified";
        break;
      }

      options.rom_file = argv[i];
    }
  }

  return options;
}
