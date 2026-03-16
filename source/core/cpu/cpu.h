#pragma once

#include "addressing_mode.h"
#include "memory.h"
#include "opcodes.h"

typedef enum {
  FETCH,
  FETCH_OPERAND,
  DECODE,
  EXECUTE,
} cpu_state_e;

typedef enum {
  OK = 0,
  ILLEGAL_OPCODE,
  SEGMENTATION_FAULT,  // write/read errors
  STACK_OVERFLOW,
  STACK_UNDERFLOW,
} trap_e;

#define OPERANDS_BUFFER_SIZE 8

typedef struct Cpu {
  byte_t reg_A, reg_X, reg_Y, reg_P;
  byte_t reg_SP;
  word_t reg_IP;
  opcode_e reg_IR;

  byte_t remaining_bytes;
  byte_t operands_buffer[OPERANDS_BUFFER_SIZE];
  byte_t operands_buffer_index;
  cpu_state_e state;
  trap_e last_trap;
} cpu_t;

typedef enum Flag {
  CARRY = 0,
  ZERO,
  INTERRUPT,
  DECIMAL,
  BREAK,
  OVERFLOW_,
  NEGATIVE,
} flag_e;

#define EMPTY_STACK_PTR 0xFF

void cpu_init(cpu_t *self, const memory_t *memory);
trap_e cpu_do_cycle(cpu_t *self, memory_t *memory);
byte_t cpu_fetch(cpu_t *self, const memory_t *memory, bool *success);

word_t cpu_resolve_first_operand(const cpu_t *self, const memory_t *memory, addressing_mode_e mode,
                                 bool *return_value_is_address);
word_t cpu_real_operand(cpu_t *cpu, const memory_t *memory, addressing_mode_e mode);

void cpu_update_zero_and_negative_flags(cpu_t *self, byte_t new_reg_value);
void cpu_update_carry_flag(cpu_t *self, word_t value);
void cpu_update_negative_flag(cpu_t *self, byte_t new_value);
void cpu_update_zero_flag(cpu_t *self, byte_t new_value);
void cpu_update_overflow_flag_in_arithmetic(cpu_t *self, byte_t result, byte_t value);

bool cpu_status_flag_is_set(const cpu_t *self, flag_e flag);
void cpu_status_flag_set(cpu_t *self, flag_e flag);
void cpu_status_flag_clear(cpu_t *self, flag_e flag);
