#pragma once

#include "base.h"
#include "memory.h"

typedef enum {
  NOOP_OPCOD = 0,
  MOVAI_OPCOD = 11,  // move to register A an immediate value
  MOVXI_OPCOD = 21,  // move to register X an immediate value
  MOVYI_OPCOD = 31,  // move to register Y an immediate value
  ADDI_OPCOD = 1,    // add an immediate to A register
  JMP_OPCOD = 3,     // jump to an address
} opcode_e;

typedef enum {
  FETCH,
  FETCH_OPERAND,
  DECODE,
  EXECUTE,
  WRITEBACK,
} cpu_state_e;

typedef struct {
  byte_t reg_A, reg_X, reg_Y;

  word_t reg_IP, reg_SP;
  byte_t remaining_bytes;
  byte_t operands_buffer[16];
  byte_t operands_buffer_index;
  cpu_state_e state;
  opcode_e reg_IR;
} cpu_t;

void cpu_init(cpu_t *self);
void cpu_do_cycle(cpu_t *self, memory_t *memory);
