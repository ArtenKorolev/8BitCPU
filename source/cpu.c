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

// private cpu functions
void cpu_exec(cpu_t *self, memory_t *memory, byte_t fetched_byte);
void cpu_add_intermediate_to_register_A(cpu_t *self, memory_t *memory);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);
void cpu_jump(cpu_t *self, memory_t *memory);

void cpu_do_cycle(cpu_t *self, memory_t *memory) {
  bool success = false;
  const byte_t byte = cpu_fetch(self, memory, &success);

  if (!success) {
    return;
  }

  cpu_exec(self, memory, byte);
}

void cpu_exec(cpu_t *self, memory_t *memory, byte_t fetched_byte) {
  printf("Opcode description: ");

  switch (fetched_byte) {
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
    case JMP_OPCOD:
      cpu_jump(self, memory);
      break;
    default:
      puts("Unknown opcode;");
      break;
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

bool validate_address(word_t address) {
  if (address < 0 || address > WORD_MAX) {
    return false;
  }

  return true;
}

void cpu_jump(cpu_t *self, memory_t *memory) {
  puts("Jump to an address;");
  bool success = false;
  printf("IP now: %d\n", self->ip);

  byte_t first_byte = cpu_fetch(self, memory, &success);

  if (!success) {
    puts("Error: Error while reading high part of address");
    return;
  }

  word_t address = first_byte;
  address <<= 8;

  byte_t second_byte = cpu_fetch(self, memory, &success);

  if (!success) {
    puts("Error: Error while reading low part of address");
    return;
  }

  address += second_byte;

  printf("Address for jumping: %d\n", address);

  if (!validate_address(address)) {
    puts("Error: Error in validating address");
    return;
  }

  self->ip = address;

  printf("IP after: %d\n", self->ip);
}

bool check_8bit_overflow(byte_t a, byte_t b) {
  return a + b > BYTE_MAX;
}

void cpu_add_intermediate_to_register_A(cpu_t *self, memory_t *memory) {
  bool success = false;

  printf("Add to register A an intermediate\n");

  const byte_t intermediate = cpu_fetch(self, memory, &success);

  printf("Register A now: %d\n", self->regA);

  if (success) {
    if (check_8bit_overflow(self->regA, intermediate)) {
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
