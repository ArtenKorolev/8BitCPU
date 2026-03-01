#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "base.h"
#include "cpu.h"
#include "file_io.h"
#include "log.h"
#include "memory.h"

#define LOG_INFO_FLAG "-linfo"
#define LOG_INFO_FLAG_LEN 6

#define LOG_WARN_FLAG "-lwarn"
#define LOG_WARN_FLAG_LEN 6

#define LOG_ERROR_FLAG "-lerr"
#define LOG_ERROR_FLAG_LEN 5

log_level_e g_log_level = NO_LOG;

#include "apple2.h"

void memory_init_apple2(memory_t *self) {
  memory_init(self, apple2_memory_read, apple2_memory_write);
}

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

  memory_t memory;
  memory_init_apple2(&memory);

  file_content_t file_content = read_bin_file("mem.bin");

  if (file_content.size == 0) {
    emu_log(ERROR, "Zero bytes of program are read\n");
    return EXIT_FAILURE;
  }

  if (file_content.size > MEMORY_SIZE) {
    memcpy(memory.memory, file_content.data, MEMORY_SIZE);
  } else {
    memcpy(memory.memory, file_content.data, file_content.size);
  }

  file_content_free(&file_content);

  cpu_t cpu;
  cpu_init(&cpu, &memory);

  for (;;) {
    const trap_e cycle_result = cpu_do_cycle(&cpu, &memory);

    if (cycle_result != OK) {
      emu_log(ERROR, "!TRAP!\nTrap number: %d\n", cycle_result);
      break;
    }

    for (int i = 0; i < 100000000; ++i);
  }

  memory_free(&memory);

  return 0;
}
