#pragma once

#include "base.h"
#include "memory.h"

typedef enum {
  NOOP_OPCOD = 0,
  MOVAI_OPCOD = 11,  // move to register A an intermediate value
  MOVXI_OPCOD = 21,  // move to register X an intermediate value
  MOVYI_OPCOD = 31,  // move to register Y an intermediate value
  MOVZI_OPCOD = 41,  // move to register Z an intermediate value
  ADDI_OPCOD = 1,    // add an intermediate to A register
  ADDR_OPCOD = 2,    // add a register's value to A register
  JMP_OPCOD = 3,     // jump to an address
} opcode_e;

typedef struct {
  byte_t regA, regX, regY, regZ;

  word_t ip, sp;
} cpu_t;

void cpu_init(cpu_t *self);
void cpu_do_cycle(cpu_t *self, memory_t *memory);
