#pragma once

#include <stdio.h>

#include "base.h"
#include "memory.h"

typedef enum {
  NOP_OPCOD = 0xEA,

  LDAI_OPCOD = 0xA9,   // move to register A an immediate value
  LDAZ_OPCOD = 0xA5,   // move to register A a value from zero page address
  LDAZX_OPCOD = 0xB5,  // move to register A a value from zero page address + X
  LDAA_OPCOD = 0xAD,   // move to register A a value from absolute address
  LDAAX_OPCOD = 0xBD,  // move to register A a value from absolute address + X
  LDAAY_OPCOD = 0xB9,  // move to register A a value from absolute address + X

  LDXI_OPCOD = 0xA2,   // move to register X an immediate value
  LDXZ_OPCOD = 0xA6,   // move to register X a value from zero page address
  LDXZY_OPCOD = 0xB6,  // move to register X a value from zero page address + Y
  LDXA_OPCOD = 0xAE,   // move to register X a value from absolute address
  LDXAY_OPCOD = 0xBE,  // move to register X a value from absolute address + Y

  LDYI_OPCOD = 0xA0,   // move to register Y an immediate value
  LDYZ_OPCOD = 0xA4,   // move to register Y a value from zero page address
  LDYZX_OPCOD = 0xB4,  // move to register Y a value from zero page address + X
  LDYA_OPCOD = 0xAC,   // move to register Y a value from absolute address
  LDYAX_OPCOD = 0xBC,  // move to register Y a value from absolute address + X

  STAZ_OPCOD = 0x85,   // store register A to address in zero page address
  STAZX_OPCOD = 0x95,  // store register A to address in zero page address + X
  STAA_OPCOD = 0x8D,   // store register A to address in absolute address
  STAAX_OPCOD = 0x9D,  // store register A to address in absolute address + X
  STAAY_OPCOD = 0x99,  // store register A to address in absolute address + Y

  STXZ_OPCOD = 0x86,   // store register X to address in zero page address
  STXZY_OPCOD = 0x96,  // store register X to address in zero page address + Y
  STXA_OPCOD = 0x8E,   // store register X to address in absolute address

  STYZ_OPCOD = 0x84,   // store register Y to address in zero page address
  STYZX_OPCOD = 0x94,  // store register Y to address in zero page address + X
  STYA_OPCOD = 0x8C,   // store register Y to address in absolute address

  ANDI_OPCOD = 0x29,   // logical and A with immediate
  ANDZ_OPCOD = 0x25,   // logical and A with a value from zero page address
  ANDZX_OPCOD = 0x35,  // logical and A with a value from zero page address + X
  ANDA_OPCOD = 0x2D,   // logical and A with a value from absolute address
  ANDAX_OPCOD = 0x3D,  // logical and A with a value from absolute address + X
  ANDAY_OPCOD = 0x39,  // logical and A with a value from absolute addres + Y

  JSR_OPCOD = 0x20,  // jump to subroutine (with saving return address)
  RTS_OPCOD = 0x60,  // return from subroutine

  ADDI_OPCOD = 0x69,  // add an immediate to A register

  JMPA_OPCOD = 0x4C,  // jump to an address

  BCC_OPCOD = 0x90,  // Branch if carry clear
  BCS_OPCOD = 0xB0,  // Branch if carry set
  BEQ_OPCOD = 0xF0,  // Branch if zero set
  BNE_OPCOD = 0xD0,  // Branch if zero clear
  BPL_OPCOD = 0x10,  // Branch if negative clear
  BMI_OPCOD = 0x30,  // Branch if negative set
  BVC_OPCOD = 0x50,  // Branch if overflow clear
  BVS_OPCOD = 0x70,  // Branch if overflow set

  CMPI_OPCOD = 0xC9,  // Compare accumulator with immediate
  CMPZ_OPCOD = 0xC5,  // Compare accumulator with zero page
  CMPA_OPCOD = 0xCD,  // Compare accumulator with absolute

  CPXI_OPCOD = 0xE0,  // Compare X with immediate
  CPXZ_OPCOD = 0xE4,  // Compare X with zero page
  CPXA_OPCOD = 0xEC,  // Compare X with absolute

  CPYI_OPCOD = 0xC0,  // Compare Y with immediate
  CPYZ_OPCOD = 0xC4,  // Compare Y with zero page
  CPYA_OPCOD = 0xCC,  // Compare Y with absolute

  CLC_OPCOD = 0x18,  // clear carry
  CLD_OPCOD = 0xD8,  // clear decimal mode
  CLI_OPCOD = 0x58,  // clear interrupt disable
  CLV_OPCOD = 0xB8,  // clear overflow
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
void cpu_dump(const cpu_t *self, FILE *stream);
