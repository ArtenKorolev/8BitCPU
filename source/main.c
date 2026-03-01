#include <string.h>

#include "emulator.h"
#include "log.h"

#define LOG_INFO_FLAG "-linfo"
#define LOG_INFO_FLAG_LEN 6

#define LOG_WARN_FLAG "-lwarn"
#define LOG_WARN_FLAG_LEN 6

#define LOG_ERROR_FLAG "-lerr"
#define LOG_ERROR_FLAG_LEN 5

log_level_e g_log_level = NO_LOG;

int main(const int argc, const char **argv) {
  for (int i = 0; i < argc; ++i) {
    if (strncmp(argv[i], LOG_INFO_FLAG, LOG_INFO_FLAG_LEN) == 0) {
      g_log_level = INFO;
    } else if (strncmp(argv[i], LOG_WARN_FLAG, LOG_WARN_FLAG_LEN) == 0) {
      g_log_level = WARN;
    } else if (strncmp(argv[i], LOG_ERROR_FLAG, LOG_ERROR_FLAG_LEN) == 0) {
      g_log_level = ERROR;
    }
  }

  emulator_t emu;
  emulator_init(&emu, APPLE2);  // hardcoded Apple II
  const int exit_code = emulator_run(&emu);
  emulator_shutdown(&emu);

  return exit_code;
}
