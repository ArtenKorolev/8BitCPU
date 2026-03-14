#include <string.h>

#include "emulator.h"
#include "log.h"
#include "utils/args.h"

log_level_e g_log_level = NO_LOG;

int main(const int argc, const char **argv) {
  parse_args(argc, argv);

  emulator_t emu;
  emulator_init(&emu, APPLE2);  // hardcoded Apple II
  const int exit_code = emulator_run(&emu);
  emulator_shutdown(&emu);

  return exit_code;
}
