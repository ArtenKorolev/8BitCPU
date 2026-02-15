#include "cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"

void cpu_init(cpu_t *self) {
  self->reg_IP = 0;
  self->reg_SP = 0;

  self->reg_A = self->reg_X = self->reg_Y = 0;

  memset(self->operands_buffer, 0, 16);
  self->operands_buffer_index = 0;
  self->state = FETCH;
}

// private cpu functions
void cpu_exec(cpu_t *self, memory_t *memory);
void cpu_move_to_register_immediate(cpu_t *self, byte_t *register_ptr, char register_name);
void cpu_add_immediate_to_register_A(cpu_t *self);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);
void cpu_jump(cpu_t *self, memory_t *memory);
void cpu_set_remaining_bytes(cpu_t *self);

void cpu_do_cycle(cpu_t *self, memory_t *memory) {
  bool success = false;

  switch (self->state) {
    case FETCH:
      self->reg_IR = cpu_fetch(self, memory, &success);

      if (!success) {  // failed fetching opcode
        self->reg_IR = 0;
        return;
      }

      self->state = FETCH_OPERAND;
      cpu_set_remaining_bytes(self);

      return;
    case FETCH_OPERAND:
      if (self->remaining_bytes == 0) {
        self->state = DECODE;
        return;
      }

      self->operands_buffer[self->operands_buffer_index++] = cpu_fetch(self, memory, &success);

      if (!success) {  // failed fetching operands
        return;
      }

      --self->remaining_bytes;
      return;
    case DECODE:
      // decoding
      self->state = EXECUTE;
      return;
    case EXECUTE:
      cpu_exec(self, memory);
      self->state = WRITEBACK;
      return;
    case WRITEBACK:
      self->state = FETCH;
      memset(self->operands_buffer, 0, 16);
      self->operands_buffer_index = 0;
      self->remaining_bytes = 0;
      return;
  }
}
void cpu_set_remaining_bytes(cpu_t *self) {
  byte_t bytes = 0;

  switch (self->reg_IR) {
    case MOVAI_OPCOD:
    case MOVXI_OPCOD:
    case MOVYI_OPCOD:
    case ADDI_OPCOD:
      bytes = 1;
      break;
    case JMP_OPCOD:
      bytes = 2;
      break;
    case NOOP_OPCOD:
      break;
  }

  self->remaining_bytes = bytes;
}

void cpu_exec(cpu_t *self, memory_t *memory) {
  printf("Opcode description: ");

  switch (self->reg_IR) {
    case NOOP_OPCOD:
      puts("No operation;");
      break;
    case MOVAI_OPCOD:
      cpu_move_to_register_immediate(self, &self->reg_A, 'A');
      break;
    case MOVXI_OPCOD:
      cpu_move_to_register_immediate(self, &self->reg_X, 'X');
      break;
    case MOVYI_OPCOD:
      cpu_move_to_register_immediate(self, &self->reg_Y, 'Y');
      break;
    case ADDI_OPCOD:
      cpu_add_immediate_to_register_A(self);
      break;
    case JMP_OPCOD:
      cpu_jump(self, memory);
      break;
    default:
      puts("Unknown opcode;");
      break;
  }
}

void cpu_move_to_register_immediate(cpu_t *self, byte_t *register_ptr, const char register_name) {
  if (register_ptr == NULL) {
    puts("register_ptr is NULL for some reason");
    return;
  }

  printf("Move to register %c an immediate;\n", register_name);
  printf("Register %c now: %d\n", register_name, *register_ptr);

  const byte_t immediate = self->operands_buffer[0];

  *register_ptr = immediate;

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
  printf("IP now: %d\n", self->reg_IP);

  byte_t first_byte = self->operands_buffer[0];

  word_t address = first_byte;
  address <<= 8;

  byte_t second_byte = self->operands_buffer[1];

  address += second_byte;

  printf("Address for jumping: %d\n", address);

  if (!validate_address(address)) {
    puts("Error: Error in validating address");
    return;
  }

  self->reg_IP = address;

  printf("IP after: %d\n", self->reg_IP);
}

bool check_8bit_overflow(byte_t a, byte_t b) {
  return a + b > BYTE_MAX;
}

void cpu_add_immediate_to_register_A(cpu_t *self) {
  printf("Add to register A an immediate\n");

  const byte_t immediate = self->operands_buffer[0];

  printf("Register A now: %d\n", self->reg_A);

  if (check_8bit_overflow(self->reg_A, immediate)) {
    puts("Error: Overflow in addition");
  } else {
    self->reg_A += immediate;
  }

  printf("Register A after: %d\n", self->reg_A);
}

byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}
