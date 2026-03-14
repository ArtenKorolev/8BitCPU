#include "cpu.h"

#include <stdbool.h>
#include <string.h>

#include "addressing_mode.h"
#include "base.h"
#include "instructions.h"
#include "log.h"
#include "memory.h"

// private cpu functions
void cpu_reset_operands_buffer(cpu_t *self);
word_t cpu_read_reset_vector(cpu_t *self, const memory_t *memory);
void cpu_exec(cpu_t *self, memory_t *memory);
void cpu_set_remaining_bytes(cpu_t *self);

#define CARRY_MASK 0x1
#define ZERO_MASK 0x2
#define INTERRUPT_MASK 0x4
#define DECIMAL_MASK 0x8
#define BREAK_MASK 0x10
#define OVERFLOW_MASK 0x40
#define NEGATIVE_MASK 0x80
#define EMPTY_STATUS 0x20

byte_t get_mask_for_flag(const flag_e flag) {
  static byte_t masks[] = {
      CARRY_MASK, ZERO_MASK, INTERRUPT_MASK, DECIMAL_MASK, BREAK_MASK, OVERFLOW_MASK, NEGATIVE_MASK, EMPTY_STATUS,
  };

  return masks[flag];
}

void cpu_init(cpu_t *self, const memory_t *memory) {
  self->reg_IP = cpu_read_reset_vector(self, memory);
  self->reg_SP = EMPTY_STACK_PTR;
  self->reg_A = self->reg_X = self->reg_Y = 0;
  self->reg_P = EMPTY_STATUS;

  cpu_reset_operands_buffer(self);
  self->state = FETCH;
  self->last_trap = OK;
}

#define RESET_VECTOR_LOW 0xFFFC
#define RESET_VECTOR_HIGH 0xFFFD

word_t cpu_read_reset_vector(cpu_t *self, const memory_t *memory) {
  bool suc = true;

  const word_t address =
      MAKE_WORD(memory_read(memory, RESET_VECTOR_HIGH, &suc), memory_read(memory, RESET_VECTOR_LOW, &suc));

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return 0;
  }

  emu_log(INFO, "Reset vector: %x\n", address);

  return address;
}

inline void cpu_reset_operands_buffer(cpu_t *self) {
  memset(self->operands_buffer, 0, OPERANDS_BUFFER_SIZE);
  self->operands_buffer_index = 0;
}

// status register utils
inline bool cpu_status_flag_is_set(const cpu_t *self, const flag_e flag) {
  return self->reg_P & get_mask_for_flag(flag);
}

inline void cpu_status_flag_set(cpu_t *self, const flag_e flag) {
  self->reg_P |= get_mask_for_flag(flag);
}

inline void cpu_status_flag_clear(cpu_t *self, const flag_e flag) {
  self->reg_P &= ~get_mask_for_flag(flag);
}

inline void cpu_update_carry_flag(cpu_t *self, const word_t value) {
  if (value > 0xFF) {
    cpu_status_flag_set(self, CARRY);
  } else {
    cpu_status_flag_clear(self, CARRY);
  }
}

inline void cpu_update_zero_flag(cpu_t *self, const byte_t new_value) {
  if (new_value == 0) {
    cpu_status_flag_set(self, ZERO);
  } else {
    cpu_status_flag_clear(self, ZERO);
  }
}

inline void cpu_update_negative_flag(cpu_t *self, const byte_t new_value) {
  if (new_value & NEGATIVE_MASK) {
    cpu_status_flag_set(self, NEGATIVE);
  } else {
    cpu_status_flag_clear(self, NEGATIVE);
  }
}

inline void cpu_update_zero_and_negative_flags(cpu_t *self, const byte_t new_reg_value) {
  cpu_update_zero_flag(self, new_reg_value);
  cpu_update_negative_flag(self, new_reg_value);
}

word_t calculate_buggy_address_of_hi_part(word_t address);

word_t cpu_resolve_first_operand(const cpu_t *self, const memory_t *memory, const addressing_mode_e mode,
                                 bool *return_value_is_address) {
  word_t value = 0;

  bool suc = true;

  if (return_value_is_address != NULL) {
    *return_value_is_address = true;
  }

  switch (mode) {
    case ACCUMULATOR:
      if (return_value_is_address != NULL) {
        *return_value_is_address = false;
      }

      value = self->reg_A;

      break;
    case IMMEDIATE:
      value = self->operands_buffer[0];

      if (return_value_is_address != NULL) {
        *return_value_is_address = false;
      }

      break;
    case RELATIVE:
      if (return_value_is_address != NULL) {
        *return_value_is_address = false;
      }

      value = self->operands_buffer[0];
      break;
    case ZERO_PAGE:
      value = self->operands_buffer[0];
      break;
    case ZERO_PAGE_X:
      value = (self->operands_buffer[0] + self->reg_X) & 0xFF;
      break;
    case ABSOLUTE:
      value = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]);
      break;
    case ABSOLUTE_X:
      value = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_X);
      break;
    case ABSOLUTE_Y:
      value = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_Y);
      break;
    case INDIRECT:
      // address of effective address
      value = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]);  // not an effective address!
      const word_t hi_part = calculate_buggy_address_of_hi_part(value);
      value = MAKE_WORD(memory_read(memory, hi_part, &suc), memory_read(memory, value, &suc));  // effective address
      break;
    case INDIRECT_Y:
      value = self->operands_buffer[0];
      value =
          (MAKE_WORD(memory_read(memory, (value + 1) & 0xFF, &suc), memory_read(memory, value, &suc)) + self->reg_Y) &
          0xFFFF;
      break;
    case INDIRECT_X:
      value = self->operands_buffer[0];
      value = (value + self->reg_X) & 0xFF;
      value = MAKE_WORD(memory_read(memory, (value + 1) & 0xFF, &suc), memory_read(memory, value, &suc)) & 0xFFFF;
      break;
    default:
      return 0;
  }

  return value;
}

inline word_t calculate_buggy_address_of_hi_part(const word_t address) {
  if ((address & 0xFF) != 0xFF) {
    return address + 1;
  }

  return address & 0xFF00;  // wrapping around the lower part of a byte
}

trap_e cpu_do_cycle(cpu_t *self, memory_t *memory) {
  bool success = false;

  emu_log(INFO, "Program counter: %x\n", self->reg_IP);

  switch (self->state) {
    case FETCH:
      self->reg_IR = cpu_fetch(self, memory, &success);
      emu_log(INFO, "Fetched opcode: %x\n", self->reg_IR);

      if (!success) {  // failed fetching opcode
        self->reg_IR = 0;
        self->last_trap = SEGMENTATION_FAULT;
        self->state = FETCH;
        break;
      }

      self->state = DECODE;

      break;
    case FETCH_OPERAND:
      if (self->remaining_bytes == 0) {
        self->state = EXECUTE;
        break;
      }

      if (self->operands_buffer_index == OPERANDS_BUFFER_SIZE) {
        self->last_trap = SEGMENTATION_FAULT;
        break;
      }

      self->operands_buffer[self->operands_buffer_index++] = cpu_fetch(self, memory, &success);

      if (!success) {
        self->last_trap = SEGMENTATION_FAULT;
        break;
      }

      --self->remaining_bytes;
      break;
    case DECODE:
      cpu_set_remaining_bytes(self);
      self->state = FETCH_OPERAND;
      break;
    case EXECUTE:
      cpu_exec(self, memory);
      self->state = FETCH;
      cpu_reset_operands_buffer(self);
      self->remaining_bytes = 0;
      break;
  }

  return self->last_trap;
}

void cpu_set_remaining_bytes(cpu_t *self) {
  const opcode_data_t *opcode_data = get_opcode_data(self->reg_IR);

  if (opcode_data == NULL) {
    self->last_trap = ILLEGAL_OPCODE;
    emu_log(ERROR, "NULL\n");
    return;
  }

  self->remaining_bytes = opcode_data->bytes_for_operands;
}

word_t cpu_real_operand(cpu_t *cpu, const memory_t *memory, const addressing_mode_e mode) {
  bool suc = true;
  bool is_address = true;

  word_t value = cpu_resolve_first_operand(cpu, memory, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    cpu->last_trap = SEGMENTATION_FAULT;
    return 0;
  }

  return value;
}

byte_t cpu_fetch(cpu_t *self, const memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}

void cpu_exec(cpu_t *self, memory_t *memory) {
  const opcode_data_t *opcode_data = get_opcode_data(self->reg_IR);

  if (opcode_data == NULL) {
    self->last_trap = ILLEGAL_OPCODE;
    emu_log(ERROR, "NULL\n");
    return;
  }

  const instr_context_t context = {self, memory, opcode_data->addressing_mode};
  opcode_data->impl(&context);
}
