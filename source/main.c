#include <stdio.h>
#include <string.h>

#include "args.h"
#include "emulator.h"
#include "log.h"
#include "options.h"

log_level_e g_log_level = NO_LOG;

int main(const int argc, const char **argv) {
  const options_t options = parse_args(argc, argv);

  if (options.error_msg != NULL) {
    print_usage();
    fprintf(stderr, "%s\n", options.error_msg);
    return 1;
  }

  if (options.empty) {
    print_usage();
    return 1;
  }

  g_log_level = options.log_level;

  emulator_t emu;
  emulator_init(&emu, APPLE2, &options);  // hardcoded Apple II
  const int exit_code = emulator_run(&emu);
  emulator_shutdown(&emu);

  return exit_code;
}
