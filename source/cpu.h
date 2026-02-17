#pragma once

#include "base.h"
#include "memory.h"

typedef enum {
  NOP_OPCOD = 0xEA,
  LDAI_OPCOD = 0xA9,   // move to register A an immediate value
  LDAZ_OPCOD = 0xA5,   // move to register A a value from zero page
  LDAZX_OPCOD = 0xB5,  // move to register A a value from zero page + X
  LDAA_OPCOD = 0xAD,   // move to register A a value from absolute address
  LDAAX_OPCOD = 0xBD,  // move to register A a value from absolute address + X
  LDAAY_OPCOD = 0xB9,  // move to register A a value from absolute address + X
  LDXI_OPCOD = 0xA2,   // move to register X an immediate value
  LDYI_OPCOD = 0xA0,   // move to register Y an immediate value
  STAZ_OPCOD = 0x85,   // store register A to address in zero page
  ADDI_OPCOD = 0x69,   // add an immediate to A register
  JMPA_OPCOD = 0x4C,   // jump to an address
} opcode_e;

typedef enum {
  IMMEDIATE,
  ACCUMULATOR,
  ZERO_PAGE,
  ZERO_PAGE_X,
  ZERO_PAGE_Y,
  RELATIVE,
  ABSOLUTE,
  ABSOLUTE_X,
  ABSOLUTE_Y,
  INDERECT,
  INDEXED_INDERECT,
  INDERECT_INDEXED,
} addressing_mode_t;

typedef enum {
  FETCH,
  FETCH_OPERAND,
  DECODE,
  EXECUTE,
  WRITEBACK,
} cpu_state_e;

typedef struct {
  byte_t reg_A, reg_X, reg_Y, reg_P;
  byte_t reg_SP;
  word_t reg_IP;

  byte_t remaining_bytes;
  byte_t operands_buffer[16];
  byte_t operands_buffer_index;
  cpu_state_e state;
  opcode_e reg_IR;
} cpu_t;

void cpu_init(cpu_t *self);
void cpu_do_cycle(cpu_t *self, memory_t *memory);
