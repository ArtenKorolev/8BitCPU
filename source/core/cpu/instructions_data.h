#pragma once

#include "addressing_mode.h"
#include "base.h"
#include "opcodes.h"

struct Cpu;
typedef struct Cpu cpu_t;

struct Memory;
typedef struct Memory memory_t;

typedef struct {
  cpu_t *cpu;
  memory_t *memory;
  addressing_mode_e mode;
} instr_context_t;

typedef void (*instr_impl_ptr_t)(const instr_context_t *ctx);

typedef struct {
  byte_t bytes_for_operands;
  instr_impl_ptr_t impl;
  addressing_mode_e addressing_mode;
} opcode_data_t;

const opcode_data_t *get_opcode_data(opcode_e opcode);
