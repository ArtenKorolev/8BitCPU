#include "cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "memory.h"

#define STACK_LOWEST_ADDRESS 0x0100

// private cpu functions
void cpu_reset_operands_buffer(cpu_t *self);
word_t cpu_read_reset_vector(cpu_t *self, const memory_t *memory);
void cpu_exec(cpu_t *self, memory_t *memory);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);
void cpu_jump(cpu_t *self, addressing_mode_e mode, const memory_t *memory);
void cpu_set_remaining_bytes(cpu_t *self);
void cpu_update_zero_and_negative_flags(cpu_t *self, const byte_t new_reg_value);
void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, addressing_mode_e mode,
                          const memory_t *memory);
void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        addressing_mode_e mode);
void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void cpu_and_with_accumulator(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
word_t cpu_resolve_first_operand(const cpu_t *self, const memory_t *memory, const addressing_mode_e mode,
                                 bool *return_value_is_address);
void cpu_jump_subroutine(cpu_t *self, memory_t *memory);
void cpu_return_from_subroutine(cpu_t *self, memory_t *memory);
void cpu_branch_based_on_flag(cpu_t *self, const byte_t mask, const bool branch_if_set);
void cpu_compare(cpu_t *self, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode);
void cpu_decrement_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode);
void cpu_increment_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode);
void cpu_decrement_register(cpu_t *self, byte_t *register_ptr);
void cpu_increment_register(cpu_t *self, byte_t *register_ptr);
void cpu_test_bit(cpu_t *self, memory_t *memory, const addressing_mode_e mode);
void cpu_transfer_registers(cpu_t *self, byte_t *first_reg, byte_t *second_reg);

#define MAKE_WORD(a, b) ((a << 8) | (b))

#define CARRY_MASK 0b00000001
#define ZERO_MASK 0x00000010
#define INTERRUPT_MASK 0b00000100
#define DECIMAL_MASK 0b00001000
#define BREAK_MASK 0b00010000
#define OVERFLOW_MASK 0b01000000
#define NEGATIVE_MASK 0b10000000

#define EMPTY_STATUS 0b00100000

void cpu_init(cpu_t *self, const memory_t *memory) {
  self->reg_IP = cpu_read_reset_vector(self, memory);
  self->reg_SP = 0xFF;
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

void cpu_reset_operands_buffer(cpu_t *self) {
  memset(self->operands_buffer, 0, OPERANDS_BUFFER_SIZE);
  self->operands_buffer_index = 0;
}

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

trap_e cpu_do_cycle(cpu_t *self, memory_t *memory) {
  bool success = false;

  switch (self->state) {
    case FETCH:
      self->reg_IR = cpu_fetch(self, memory, &success);

      if (!success) {  // failed fetching opcode
        self->reg_IR = 0;
        self->last_trap = SEGMENTATION_FAULT;
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
      self->state = WRITEBACK;
      break;
    case WRITEBACK:
      self->state = FETCH;
      cpu_reset_operands_buffer(self);
      self->remaining_bytes = 0;
      break;
  }

  return self->last_trap;
}

void cpu_set_remaining_bytes(cpu_t *self) {
  byte_t bytes = 0;

  switch (self->reg_IR) {
    case LDAI_OPCOD:
    case SEC_OPCOD:
    case SEI_OPCOD:
    case SED_OPDCOD:
    case LDXI_OPCOD:
    case LDYI_OPCOD:
    case ADDI_OPCOD:
    case LDAZ_OPCOD:
    case LDAZX_OPCOD:
    case STAZ_OPCOD:
    case LDXZ_OPCOD:
    case LDYZ_OPCOD:
    case BITZ_OPCOD:
    case LDYZX_OPCOD:
    case STYZ_OPCOD:
    case STYZX_OPCOD:
    case STXZ_OPCOD:
    case STXZY_OPCOD:
    case STAZX_OPCOD:
    case LDXZY_OPCOD:
    case ANDI_OPCOD:
    case ANDZX_OPCOD:
    case ANDZ_OPCOD:
    case BCC_OPCOD:
    case BCS_OPCOD:
    case BEQ_OPCOD:
    case DECZX_OPCOD:
    case DECZ_OPCOD:
    case BMI_OPCOD:
    case BNE_OPCOD:
    case BPL_OPCOD:
    case BVC_OPCOD:
    case BVS_OPCOD:
    case CMPI_OPCOD:
    case CMPZ_OPCOD:
    case CPYI_OPCOD:
    case CPYZ_OPCOD:
    case CPXI_OPCOD:
    case CPXZ_OPCOD:
    case INCZ_OPCOD:
    case INCZX_OPCOD:
    case ADDZ_OPCOD:
    case ADDZX_OPCOD:
    case LDAIX_OPCOD:
    case LDAIY_OPCOD:
    case STAIX_OPCOD:
    case STAIY_OPCOD:
      bytes = 1;
      break;
    case DECA_OPCOD:
    case DECAX_OPCOD:
    case INCA_OPCOD:
    case INCAX_OPCOD:
    case JMPA_OPCOD:
    case LDAA_OPCOD:
    case LDAAX_OPCOD:
    case LDAAY_OPCOD:
    case LDYA_OPCOD:
    case LDXA_OPCOD:
    case LDYAX_OPCOD:
    case LDXAY_OPCOD:
    case STAAX_OPCOD:
    case STAA_OPCOD:
    case STAAY_OPCOD:
    case STXA_OPCOD:
    case STYA_OPCOD:
    case ANDA_OPCOD:
    case ANDAX_OPCOD:
    case ANDAY_OPCOD:
    case JSR_OPCOD:
    case CMPA_OPCOD:
    case CPXA_OPCOD:
    case BITA_OPCOD:
    case CPYA_OPCOD:
    case ADDA_OPCOD:
    case ADDAY_OPCOD:
    case JMPI_OPCOD:
    case ADDAX_OPCOD:
      bytes = 2;
      break;
    case NOP_OPCOD:
    case DEX_OPCOD:
    case DEY_OPCOD:
    case CLC_OPCOD:
    case CLD_OPCOD:
    case CLI_OPCOD:
    case CLV_OPCOD:
    case RTS_OPCOD:
    case INY_OPCOD:
    case INX_OPCOD:
    case TXA_OPCOD:
    case TAX_OPCOD:
    case TAY_OPCOD:
    case TYA_OPCOD:
    case TXS_OPCOD:
    case TSX_OPCOD:
      break;
  }

  self->remaining_bytes = bytes;
}

void cpu_exec(cpu_t *self, memory_t *memory) {
  switch ((opcode_e)self->reg_IR) {
    case NOP_OPCOD:
      emu_log(INFO, "No operation;\n");
      break;
    case TAY_OPCOD:
      cpu_transfer_registers(self, &self->reg_A, &self->reg_Y);
      break;
    case TAX_OPCOD:
      cpu_transfer_registers(self, &self->reg_A, &self->reg_X);
      break;
    case TSX_OPCOD:
      cpu_transfer_registers(self, &self->reg_SP, &self->reg_X);
      break;
    case TXS_OPCOD:
      cpu_transfer_registers(self, &self->reg_X, &self->reg_SP);
      break;
    case STAIX_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, INDEXED_INDERECT_X);
      break;
    case STAIY_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, INDERECT_INDEXED_Y);
      break;
    case TXA_OPCOD:
      cpu_transfer_registers(self, &self->reg_X, &self->reg_A);
      break;
    case TYA_OPCOD:
      cpu_transfer_registers(self, &self->reg_Y, &self->reg_A);
      break;
    case INY_OPCOD:
      cpu_increment_register(self, &self->reg_Y);
      break;
    case INX_OPCOD:
      cpu_increment_register(self, &self->reg_X);
      break;
    case INCAX_OPCOD:
      cpu_increment_memory(self, memory, ABSOLUTE_X);
      break;
    case INCA_OPCOD:
      cpu_increment_memory(self, memory, ABSOLUTE);
      break;
    case INCZ_OPCOD:
      cpu_increment_memory(self, memory, ZERO_PAGE);
      break;
    case INCZX_OPCOD:
      cpu_increment_memory(self, memory, ZERO_PAGE_X);
      break;
    case BITA_OPCOD:
      cpu_test_bit(self, memory, ABSOLUTE);
      break;
    case BITZ_OPCOD:
      cpu_test_bit(self, memory, ZERO_PAGE);
      break;
    case DEX_OPCOD:
      cpu_decrement_register(self, &self->reg_X);
      break;
    case DEY_OPCOD:
      cpu_decrement_register(self, &self->reg_Y);
      break;
    case DECA_OPCOD:
      cpu_decrement_memory(self, memory, ABSOLUTE);
      break;
    case DECAX_OPCOD:
      cpu_decrement_memory(self, memory, ABSOLUTE_X);
      break;
    case DECZ_OPCOD:
      cpu_decrement_memory(self, memory, ZERO_PAGE);
      break;
    case DECZX_OPCOD:
      cpu_decrement_memory(self, memory, ZERO_PAGE_X);
      break;
    case LDAI_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', IMMEDIATE, memory);
      break;
    case CLC_OPCOD:
      cpu_status_flag_clear(self, CARRY_MASK);
      break;
    case CLV_OPCOD:
      cpu_status_flag_clear(self, OVERFLOW_MASK);
      break;
    case CLI_OPCOD:
      cpu_status_flag_clear(self, INTERRUPT_MASK);
      break;
    case CLD_OPCOD:
      cpu_status_flag_clear(self, DECIMAL_MASK);
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
    case LDXZ_OPCOD:
      cpu_load_to_register(self, &self->reg_X, 'X', ZERO_PAGE, memory);
      break;
    case ADDZ_OPCOD:
      cpu_add_to_accumulator(self, memory, ZERO_PAGE);
      break;
    case ADDZX_OPCOD:
      cpu_add_to_accumulator(self, memory, ZERO_PAGE_X);
      break;
    case ADDAX_OPCOD:
      cpu_add_to_accumulator(self, memory, ABSOLUTE_X);
      break;
    case ADDAY_OPCOD:
      cpu_add_to_accumulator(self, memory, ABSOLUTE_Y);
      break;
    case ADDA_OPCOD:
      cpu_add_to_accumulator(self, memory, ABSOLUTE);
      break;
    case LDXZY_OPCOD:
      cpu_load_to_register(self, &self->reg_X, 'X', ZERO_PAGE_Y, memory);
      break;
    case LDXA_OPCOD:
      cpu_load_to_register(self, &self->reg_X, 'X', ABSOLUTE, memory);
      break;
    case LDXAY_OPCOD:
      cpu_load_to_register(self, &self->reg_X, 'X', ABSOLUTE_Y, memory);
      break;
    case LDYZ_OPCOD:
      cpu_load_to_register(self, &self->reg_Y, 'Y', ZERO_PAGE, memory);
      break;
    case LDYZX_OPCOD:
      cpu_load_to_register(self, &self->reg_Y, 'Y', ZERO_PAGE_X, memory);
      break;
    case LDYA_OPCOD:
      cpu_load_to_register(self, &self->reg_Y, 'Y', ABSOLUTE, memory);
      break;
    case LDYAX_OPCOD:
      cpu_load_to_register(self, &self->reg_Y, 'Y', ABSOLUTE_X, memory);
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
    case STAZX_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, ZERO_PAGE_X);
      break;
    case STAA_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, ABSOLUTE);
      break;
    case STAAX_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, ABSOLUTE_X);
      break;
    case STAAY_OPCOD:
      cpu_store_register(self, self->reg_A, 'A', memory, ABSOLUTE_Y);
      break;
    case STXA_OPCOD:
      cpu_store_register(self, self->reg_X, 'X', memory, ABSOLUTE);
      break;
    case STXZ_OPCOD:
      cpu_store_register(self, self->reg_X, 'X', memory, ZERO_PAGE);
      break;
    case STXZY_OPCOD:
      cpu_store_register(self, self->reg_X, 'X', memory, ZERO_PAGE_Y);
      break;
    case STYA_OPCOD:
      cpu_store_register(self, self->reg_Y, 'Y', memory, ABSOLUTE);
      break;
    case STYZ_OPCOD:
      cpu_store_register(self, self->reg_Y, 'Y', memory, ZERO_PAGE);
      break;
    case STYZX_OPCOD:
      cpu_store_register(self, self->reg_Y, 'Y', memory, ZERO_PAGE_X);
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
      cpu_jump(self, ABSOLUTE, NULL);
      break;
    case ANDA_OPCOD:
      cpu_and_with_accumulator(self, memory, ABSOLUTE);
      break;
    case ANDAX_OPCOD:
      cpu_and_with_accumulator(self, memory, ABSOLUTE_X);
      break;
    case ANDAY_OPCOD:
      cpu_and_with_accumulator(self, memory, ABSOLUTE_Y);
      break;
    case ANDI_OPCOD:
      cpu_and_with_accumulator(self, memory, IMMEDIATE);
      break;
    case ANDZ_OPCOD:
      cpu_and_with_accumulator(self, memory, ZERO_PAGE);
      break;
    case ANDZX_OPCOD:
      cpu_and_with_accumulator(self, memory, ZERO_PAGE_X);
      break;
    case JSR_OPCOD:
      cpu_jump_subroutine(self, memory);
      break;
    case RTS_OPCOD:
      cpu_return_from_subroutine(self, memory);
      break;
    case BCC_OPCOD:
      cpu_branch_based_on_flag(self, CARRY_MASK, false);
      break;
    case BCS_OPCOD:
      cpu_branch_based_on_flag(self, CARRY_MASK, true);
      break;
    case BEQ_OPCOD:
      cpu_branch_based_on_flag(self, ZERO_MASK, true);
      break;
    case BNE_OPCOD:
      cpu_branch_based_on_flag(self, ZERO_MASK, false);
      break;
    case BPL_OPCOD:
      cpu_branch_based_on_flag(self, NEGATIVE_MASK, false);
      break;
    case BMI_OPCOD:
      cpu_branch_based_on_flag(self, NEGATIVE_MASK, true);
      break;
    case SEI_OPCOD:
      cpu_status_flag_set(self, INTERRUPT_MASK);
      break;
    case SED_OPDCOD:
      cpu_status_flag_set(self, DECIMAL_MASK);
      break;
    case SEC_OPCOD:
      cpu_status_flag_set(self, CARRY_MASK);
      break;
    case BVC_OPCOD:
      cpu_branch_based_on_flag(self, OVERFLOW_MASK, false);
      break;
    case BVS_OPCOD:
      cpu_branch_based_on_flag(self, OVERFLOW_MASK, true);
      break;
    case JMPI_OPCOD:
      cpu_jump(self, ABSOLUTE, memory);
      break;
    case CMPI_OPCOD:
      cpu_compare(self, memory, self->reg_A, IMMEDIATE);
      break;
    case CMPZ_OPCOD:
      cpu_compare(self, memory, self->reg_A, ZERO_PAGE);
      break;
    case CMPA_OPCOD:
      cpu_compare(self, memory, self->reg_A, ABSOLUTE);
      break;
    case CPXI_OPCOD:
      cpu_compare(self, memory, self->reg_X, IMMEDIATE);
      break;
    case CPXZ_OPCOD:
      cpu_compare(self, memory, self->reg_X, ZERO_PAGE);
      break;
    case CPXA_OPCOD:
      cpu_compare(self, memory, self->reg_X, ABSOLUTE);
      break;
    case CPYI_OPCOD:
      cpu_compare(self, memory, self->reg_Y, IMMEDIATE);
      break;
    case CPYZ_OPCOD:
      cpu_compare(self, memory, self->reg_Y, ZERO_PAGE);
      break;
    case CPYA_OPCOD:
      cpu_compare(self, memory, self->reg_Y, ABSOLUTE);
      break;
    case LDAIX_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', INDEXED_INDERECT_X, memory);
      break;
    case LDAIY_OPCOD:
      cpu_load_to_register(self, &self->reg_A, 'A', INDERECT_INDEXED_Y, memory);
      break;
    default:
      self->last_trap = ILLEGAL_OPCODE;
      emu_log(ERROR, "Illegal opcode: %x;\n", self->reg_IR);
  }
}

void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, const addressing_mode_e mode,
                          const memory_t *memory) {
  emu_log(INFO, "Load to register;\n");
  bool suc = true;
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(self, NULL, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, first_operand, &suc);

    switch (mode) {
      case INDERECT_INDEXED_Y:
      case INDEXED_INDERECT_X:
        value = memory_read(memory, value, &suc);  // value = effective address
      default:
        break;
    }
  } else {
    value = first_operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value);
  *register_ptr = value;
}

void cpu_and_with_accumulator(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Logical AND with accumulator;\n");
  bool suc = true;
  bool is_address = true;

  const word_t operand = cpu_resolve_first_operand(self, NULL, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, operand, &suc);
  } else {
    value = operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  self->reg_A &= value;

  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        const addressing_mode_e mode) {
  emu_log(INFO, "Store register;\n");

  const word_t address = cpu_resolve_first_operand(self, NULL, mode, NULL);

  memory_write(memory, address, register_value);
}

void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Add to accumulator;\n");

  bool suc = true;
  bool is_address = true;
  word_t value = cpu_resolve_first_operand(self, NULL, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
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
  cpu_dump(self, stdout);
  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

word_t calculate_buggy_address_of_hi_part(const word_t inderect_address) {
  if ((inderect_address & 0xFF) != 0xFF) {
    return inderect_address + 1;
  }

  return inderect_address & 0xFF00;
}

word_t cpu_resolve_first_operand(const cpu_t *self, const memory_t *memory, const addressing_mode_e mode,
                                 bool *return_value_is_address) {
  word_t value = 0;

  bool suc = true;

  if (return_value_is_address != NULL) {
    *return_value_is_address = true;
  }

  switch (mode) {
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
    case INDERECT:
      // address of effective address
      value = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]);  // not an effective address!
      const word_t hi_part = calculate_buggy_address_of_hi_part(value);
      value = MAKE_WORD(memory_read(memory, hi_part, &suc), memory_read(memory, value, &suc));  // effective address
    case INDERECT_INDEXED_Y:
      value = self->operands_buffer[0];
      value = MAKE_WORD(memory_read(memory, value, &suc), memory_read(memory, (value + 1) & 0xFF, &suc)) + self->reg_Y;
      break;
    case INDEXED_INDERECT_X:
      value = self->operands_buffer[0];
      value = (value + self->reg_X) & 0xFF;
      value = MAKE_WORD(memory_read(memory, value, &suc), memory_read(memory, (value + 1) & 0xFF, &suc));
      break;
    default:
      return 0;
  }

  return value;
}

void cpu_update_zero_and_negative_flags(cpu_t *self, const byte_t new_reg_value) {
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

void cpu_push_value_onto_stack(cpu_t *self, memory_t *memory, const byte_t value) {
  emu_log(INFO, "Pushing onto the stack;\n");

  if (self->reg_SP == 0) {
    emu_log(ERROR, "Stack underflow;\n");
    self->last_trap = STACK_OVERFLOW;
    return;
  }

  memory_write(memory, STACK_LOWEST_ADDRESS + self->reg_SP--, value);
}

byte_t cpu_pull_from_stack(cpu_t *self, memory_t *memory) {
  emu_log(INFO, "Pulling from the stack;\n");

  bool suc = true;

  if (self->reg_SP == 0xFF) {
    emu_log(ERROR, "Stack underflow;\n");
    self->last_trap = STACK_UNDERFLOW;
    return 0;
  }

  const byte_t value = memory_read(memory, STACK_LOWEST_ADDRESS + ++self->reg_SP, &suc);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return 0;
  }

  return value;
}

void cpu_jump_subroutine(cpu_t *self, memory_t *memory) {
  emu_log(INFO, "Jumping to subroutine;\n");

  const word_t jumping_address = cpu_resolve_first_operand(self, NULL, ABSOLUTE, NULL);

  const word_t pushing_address =
      self->reg_IP - 1;  // address of the next instruction to execute after the subroutine call

  cpu_push_value_onto_stack(self, memory, (pushing_address >> 8) & 0xFF);  // high
  cpu_push_value_onto_stack(self, memory, pushing_address & 0xFF);         // low

  self->reg_IP = jumping_address;
}

void cpu_return_from_subroutine(cpu_t *self, memory_t *memory) {
  emu_log(INFO, "Return from subroutine;\n");

  word_t address = cpu_pull_from_stack(self, memory);
  address += (cpu_pull_from_stack(self, memory) << 8);
  self->reg_IP = address + 1;
}

void cpu_jump(cpu_t *self, const addressing_mode_e mode, const memory_t *memory) {
  emu_log(INFO, "Jump;\n");

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  self->reg_IP = address;
}

byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}

void cpu_branch_based_on_flag(cpu_t *self, const byte_t mask, const bool branch_if_set) {
  emu_log(INFO, "Branching;\n");

  bool suc = true;

  const byte_t offset = cpu_resolve_first_operand(self, NULL, RELATIVE, NULL);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const bool flag_is_set = cpu_status_flag_is_set(self, mask);

  if ((branch_if_set && flag_is_set) || (!branch_if_set && !flag_is_set)) {
    emu_log(INFO, "Condition is satisfied, jump;\n");
    self->reg_IP += (int8_t)offset;
  } else {
    emu_log(INFO, "Condition is not satisfied;\n");
  }
}

void cpu_compare(cpu_t *self, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode) {
  emu_log(INFO, "Comparing;\n");

  bool suc = true;
  bool is_address = true;
  word_t value = cpu_resolve_first_operand(self, NULL, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t result = (byte_t)(register_value - (byte_t)value);
  cpu_update_zero_and_negative_flags(self, result);

  if (register_value >= value) {
    cpu_status_flag_set(self, CARRY_MASK);
  } else {
    cpu_status_flag_clear(self, CARRY_MASK);
  }
}

void cpu_decrement_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Decrement memory;\n");

  bool suc = true;

  const word_t address = cpu_resolve_first_operand(self, NULL, mode, NULL);

  emu_log(INFO, "Decrementing address: %x\n", address);

  const byte_t value = memory_read(memory, address, &suc) - 1;

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value);

  memory_write(memory, address, value);
}

void cpu_decrement_register(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Decrement register;\n");
  --(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}

void cpu_increment_register(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Increment register;\n");
  ++(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}

void cpu_increment_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Increment memory;\n");

  const word_t address = cpu_resolve_first_operand(self, NULL, mode, NULL);

  emu_log(INFO, "Incrementing address: %x\n", address);

  bool suc = true;
  const byte_t value = memory_read(memory, address, &suc) + 1;

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value + 1);

  memory_write(memory, address, value);
}

void cpu_test_bit(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Test bit;\n");

  bool suc = true;

  const word_t address = cpu_resolve_first_operand(self, NULL, mode, NULL);

  const byte_t value = memory_read(memory, address, &suc);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t result = self->reg_A & value;

  if (result == 0) {
    cpu_status_flag_set(self, ZERO_MASK);
  } else {
    cpu_status_flag_clear(self, ZERO_MASK);
  }

  if (value & NEGATIVE_MASK) {
    cpu_status_flag_set(self, NEGATIVE_MASK);
  } else {
    cpu_status_flag_clear(self, NEGATIVE_MASK);
  }

  if (value & OVERFLOW_MASK) {
    cpu_status_flag_set(self, OVERFLOW_MASK);
  } else {
    cpu_status_flag_clear(self, OVERFLOW_MASK);
  }
}

void cpu_transfer_registers(cpu_t *self, byte_t *from, byte_t *to) {
  emu_log(INFO, "Transfering registers;");

  if (from == NULL || to == NULL) {
    return;
  }

  *to = *from;
  cpu_update_zero_and_negative_flags(self, *to);
}

void cpu_dump_one_flag(const cpu_t *self, const char *flag_name, const byte_t mask, FILE *stream) {
  if (cpu_status_flag_is_set(self, mask)) {
    fprintf(stream, "\t%s flag is set\n", flag_name);
  } else {
    fprintf(stream, "\t%s flag is clear\n", flag_name);
  }
}

void cpu_dump_processor_status(const cpu_t *self, FILE *stream) {
  switch (g_log_level) {
    case NO_LOG:
    case ERROR:
    case WARN:
      return;
  }

  cpu_dump_one_flag(self, "Carry", CARRY_MASK, stream);
  cpu_dump_one_flag(self, "Zero", ZERO_MASK, stream);
  cpu_dump_one_flag(self, "Interrupt", INTERRUPT_MASK, stream);
  cpu_dump_one_flag(self, "Decimal", DECIMAL_MASK, stream);
  cpu_dump_one_flag(self, "Break", BREAK_MASK, stream);
  cpu_dump_one_flag(self, "Overflow", OVERFLOW_MASK, stream);
  cpu_dump_one_flag(self, "Negative", NEGATIVE_MASK, stream);
}

void cpu_dump(const cpu_t *self, FILE *stream) {
  switch (g_log_level) {
    case NO_LOG:
    case ERROR:
    case WARN:
      return;
  }

  fputs("======= Dumping CPU =======\n", stream);

  fprintf(stream, "\tProgram counter (instruction pointer): %x\n", self->reg_IP);
  fprintf(stream, "\tStack pointer: %x\n", self->reg_SP);
  fprintf(stream, "\tInstruction register: %x\n", self->reg_IR);
  fprintf(stream, "\tAccumulator: %x\n", self->reg_A);
  fprintf(stream, "\tIndex register X: %x\n", self->reg_X);
  fprintf(stream, "\tIndex register Y: %x\n", self->reg_Y);
  cpu_dump_processor_status(self, stream);

  fputs("===== End dumping CPU =====\n", stream);
}
