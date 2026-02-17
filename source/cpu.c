#include "cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"

#define STACK_LOWEST_ADDRESS 0x0100

void cpu_init(cpu_t *self) {
  self->reg_IP = 0;
  self->reg_SP = 0xFF;
  self->reg_A = self->reg_X = self->reg_Y = 0;

  memset(self->operands_buffer, 0, 16);
  self->operands_buffer_index = 0;
  self->state = FETCH;
}

// private cpu functions
void cpu_exec(cpu_t *self, memory_t *memory);
void cpu_add_immediate_to_register_A(cpu_t *self);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);
void cpu_jump(cpu_t *self);
void cpu_set_remaining_bytes(cpu_t *self);
void cpu_update_flags_when_loading_register(cpu_t *self, const byte_t new_reg_value);

// new
void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, addressing_mode_t mode,
                          const memory_t *memory);
void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        addressing_mode_t mode);
void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, addressing_mode_t mode);

#define MAKE_WORD(a, b) ((a << 8) | (b))

#define CARRY_MASK 0x1
#define ZERO_MASK 0x2
#define INTERRUPT_MAKS 0x4
#define DECIMAL_MAKS 0x8
#define BREAK_MASK 0x10
#define OVERFLOW_MASK 0x40
#define NEGATIVE_MASK 0x80

// status register utils
bool cpu_status_flag_is_set(const cpu_t *self, const byte_t mask) {
  return self->reg_P & mask;
}

void cpu_status_flag_set(cpu_t *self, const byte_t mask) {
  self->reg_P |= mask;
}

void cpu_status_flag_clear(cpu_t *self, const byte_t mask) {
  self->reg_P &= ~mask;
}

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
    case LDAI_OPCOD:
    case LDXI_OPCOD:
    case LDYI_OPCOD:
    case ADDI_OPCOD:
    case LDAZ_OPCOD:
    case LDAZX_OPCOD:
    case STAZ_OPCOD:
      bytes = 1;
      break;
    case JMPA_OPCOD:
    case LDAA_OPCOD:
    case LDAAX_OPCOD:
    case LDAAY_OPCOD:
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
    case LDAI_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', IMMEDIATE, memory);
      break;
    case LDAZ_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', ZERO_PAGE, memory);
      break;
    case LDAA_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', ABSOLUTE, memory);
      break;
    case LDAAX_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', ABSOLUTE_X, memory);
      break;
    case LDAAY_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', ABSOLUTE_Y, memory);
      break;
    case LDAZX_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', ZERO_PAGE_X, memory);
      break;
    case STAZ_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, ZERO_PAGE);
      break;
    case LDXI_OPCOD:
      cpu_load_to_register(self, &self->reg_X, 'X', IMMEDIATE, memory);
      break;
    case LDYI_OPCOD:
      cpu_load_to_register(self, &self->reg_Y, 'Y', IMMEDIATE, memory);
      break;
    case ADDI_OPCOD:
      cpu_add_to_accumulator(self, memory, IMMEDIATE);
      break;
    case JMPA_OPCOD:
      cpu_jump(self);
      break;
    default:
      puts("Unknown opcode;");
      break;
  }
}

void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, const addressing_mode_t mode,
                          const memory_t *memory) {
  byte_t value = 0;
  bool suc = true;

  switch (mode) {
    case IMMEDIATE:
      value = self->operands_buffer[0];
      break;
    case ZERO_PAGE:
      value = memory_read(memory, self->operands_buffer[0], &suc);
      break;
    case ZERO_PAGE_X:
      value = memory_read(memory, (self->operands_buffer[0] + self->reg_X) & 0xFF, &suc);
      break;
    case ABSOLUTE:
      value = memory_read(memory, MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) & 0xFFFF, &suc);
      break;
    case ABSOLUTE_X:
      value = memory_read(memory,
                          (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_X) & 0xFFFF, &suc);
      break;
    default:
      return;
  }

  if (!suc) {
    return;
  }

  cpu_update_flags_when_loading_register(self, value);
  *register_ptr = value;
}

void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        addressing_mode_t mode) {
  word_t address = 0x0000;

  switch (mode) {
    case ZERO_PAGE:
      address = (word_t)self->operands_buffer[0];
      break;
    case ZERO_PAGE_X:
      address = ((word_t)self->operands_buffer[0] + self->reg_X) & 0xFF;
      break;
    case ABSOLUTE:
      address = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) & 0xFFFF;
      break;
    case ABSOLUTE_X:
      address = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_X) & 0xFFFF;
      break;
    case ABSOLUTE_Y:
      address = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_Y) & 0xFFFF;
      break;
    default:
      return;
  }

  memory_write(memory, address, register_value);
}

void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, const addressing_mode_t mode) {
  byte_t value = 0;
  bool suc = true;

  switch (mode) {
    case IMMEDIATE:
      value = self->operands_buffer[0];
      break;
    case ZERO_PAGE:
      value = memory_read(memory, self->operands_buffer[0], &suc);
      break;
    case ZERO_PAGE_X:
      value = memory_read(memory, (self->operands_buffer[0] + self->reg_X) & 0xFF, &suc);
      break;
    case ABSOLUTE:
      value = memory_read(memory, MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) & 0xFFFF, &suc);
      break;
    case ABSOLUTE_X:
      value = memory_read(memory,
                          (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_X) & 0xFFFF, &suc);
      break;
    case ABSOLUTE_Y:
      value = memory_read(memory,
                          (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_Y) & 0xFFFF, &suc);
      break;
    default:
      return;
  }

  const byte_t carry = cpu_status_flag_is_set(self, CARRY_MASK) ? 1 : 0;
  const word_t result = value + self->reg_A + carry;

  if (result > 0xFF) {
    cpu_status_flag_set(self, CARRY_MASK);
  } else {
    cpu_status_flag_clear(self, CARRY_MASK);
  }

  self->reg_A = (byte_t)result;
  cpu_update_flags_when_loading_register(self, self->reg_A);
}

void cpu_update_flags_when_loading_register(cpu_t *self, const byte_t new_reg_value) {
  if (new_reg_value == 0) {
    cpu_status_flag_set(self, ZERO_MASK);
  } else {
    cpu_status_flag_clear(self, ZERO_MASK);
  }

  if ((new_reg_value & NEGATIVE_MASK) != 0) {
    cpu_status_flag_set(self, NEGATIVE_MASK);
  } else {
    cpu_status_flag_clear(self, NEGATIVE_MASK);
  }
}

bool validate_address(const word_t address) {
  if (address < 0 || address > WORD_MAX) {
    return false;
  }

  return true;
}

void cpu_jump(cpu_t *self) {
  puts("Jump to an address;");
  printf("IP now: %d\n", self->reg_IP);

  const word_t address = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]);  // little endianess

  printf("Address for jumping: %d\n", address);

  if (!validate_address(address)) {
    puts("Error: Error in validating address");
    return;
  }

  self->reg_IP = address;

  printf("IP after: %d\n", self->reg_IP);
}

byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}
