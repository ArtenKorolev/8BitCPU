#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "base.h"
#include "cpu.h"
#include "file_io.h"
#include "memory.h"

int main(const int argc, const char **argv) {
  memory_t memory;
  memory_init(&memory);

  cpu_t cpu;
  cpu_init(&cpu, &memory);

  file_content_t file_content = read_bin_file("mem.bin");

  if (file_content.size == 0) {
    puts("Zero bytes read");
  }

  if (file_content.size > MEMORY_SIZE) {
    memcpy(memory.memory, file_content.data, MEMORY_SIZE);
  } else {
    memcpy(memory.memory, file_content.data, file_content.size);
  }

  file_content_free(&file_content);

  for (;;) {
    const trap_e cycle_result = cpu_do_cycle(&cpu, &memory);

    if (cycle_result != OK) {
      puts("TRAP");
      printf("Number: %d", cycle_result);
      break;
    }

    for (int i = 0; i < 100000000; ++i);
  }

  memory_free(&memory);

  return 0;
}
