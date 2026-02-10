#include <stdio.h>

#include "cpu.h"
#include "memory.h"

int main(const int argc, const char **argv) {
  cpu_t cpu;
  cpu_init(&cpu);
  cpu.ip = 0;

  memory_t memory;
  memory_init(&memory);

  if (memory.memory_size == 0) {
    puts("Bad alloc");
    return 1;
  }

  memory.memory[1] = MOVAI_OPCOD;
  memory.memory[2] = 222;

  memory.memory[3] = MOVXI_OPCOD;
  memory.memory[4] = 221;

  memory.memory[5] = MOVYI_OPCOD;
  memory.memory[6] = 232;

  memory.memory[7] = MOVZI_OPCOD;
  memory.memory[8] = 22;

  memory.memory[9] = ADDI_OPCOD;
  memory.memory[10] = 255;

  for (int i = 0; i < 10; ++i) {
    cpu_do_cycle(&cpu, &memory);
  }

  return 0;
}
