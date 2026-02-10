#include "cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"

#define IP_START 0x7f  // magic

void cpu_init(cpu_t *self) {
  self->ip = IP_START;
  self->sp = 0;

  self->regA = self->regX = self->regY = self->regZ = 0;
}

void cpu_move_to_register_intermediate(cpu_t *self, byte_t *register_ptr,
                                       char register_name, memory_t *memory);

void cpu_add_intermediate_to_register_A(cpu_t *self, memory_t *memory);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);

void cpu_do_cycle(cpu_t *self, memory_t *memory) {
  bool success = false;
  const byte_t byte = cpu_fetch(self, memory, &success);

  if (!success) {
    return;
  }

  printf("Opcode description: ");

  switch (byte) {
    case NOOP_OPCOD:
      puts("No operation;");
      break;
    case MOVAI_OPCOD:
      cpu_move_to_register_intermediate(self, &self->regA, 'A', memory);
      break;
    case MOVXI_OPCOD:
      cpu_move_to_register_intermediate(self, &self->regX, 'X', memory);
      break;
    case MOVYI_OPCOD:
      cpu_move_to_register_intermediate(self, &self->regY, 'Y', memory);
      break;
    case MOVZI_OPCOD:
      cpu_move_to_register_intermediate(self, &self->regZ, 'Z', memory);
      break;
    case ADDI_OPCOD:
      cpu_add_intermediate_to_register_A(self, memory);
      break;
    default:
      puts("Unknown opcode;");
  }
}

void cpu_move_to_register_intermediate(cpu_t *self, byte_t *register_ptr,
                                       const char register_name,
                                       memory_t *memory) {
  if (register_ptr == NULL) {
    puts("register_ptr is NULL for some reason");
    return;
  }

  bool success = false;

  printf("Move to register %c an intermediate;\n", register_name);
  printf("Register %c now: %d\n", register_name, *register_ptr);

  const byte_t intermediate = cpu_fetch(self, memory, &success);

  if (success) {
    *register_ptr = intermediate;
  }

  printf("Register %c after: %d\n", register_name, *register_ptr);
}

void cpu_add_intermediate_to_register_A(cpu_t *self, memory_t *memory) {
  bool success = false;

  printf("Add to register A an intermediate\n");

  const byte_t intermediate = cpu_fetch(self, memory, &success);

  printf("Register A now: %d\n", self->regA);

  if (success) {
    if (self->regA + intermediate > BYTE_MAX) {
      puts("Error: Overflow in addition");
    } else {
      self->regA += intermediate;
    }
  }

  printf("Register A after: %d\n", self->regA);
}

byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success) {
  return memory_read(memory, self->ip++, success);
}
