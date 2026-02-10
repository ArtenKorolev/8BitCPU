#include <stdio.h>
#include <string.h>

#include "base.h"
#include "cpu.h"
#include "file_io.h"
#include "memory.h"

int main(const int argc, const char **argv) {
  cpu_t cpu;
  cpu_init(&cpu);
  cpu.ip = 0;

  memory_t memory;
  memory_init(&memory);

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

  while (cpu.ip < 10) {
    cpu_do_cycle(&cpu, &memory);
  }

  memory_free(&memory);

  return 0;
}
