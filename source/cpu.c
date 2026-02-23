#include "cpu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"

#define STACK_LOWEST_ADDRESS 0x0100

void cpu_reset_operands_buffer(cpu_t *self);

void cpu_init(cpu_t *self) {
  self->reg_IP = 0;
  self->reg_SP = 0xFF;
  self->reg_A = self->reg_X = self->reg_Y = 0;

  cpu_reset_operands_buffer(self);
  self->state = FETCH;
  self->last_trap = OK;
}

void cpu_reset_operands_buffer(cpu_t *self) {
  memset(self->operands_buffer, 0, OPERANDS_BUFFER_SIZE);
  self->operands_buffer_index = 0;
}

// private cpu functions
void cpu_exec(cpu_t *self, memory_t *memory);
void cpu_add_immediate_to_register_A(cpu_t *self);
byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success);
void cpu_jump(cpu_t *self);
void cpu_set_remaining_bytes(cpu_t *self);
void cpu_update_flags_when_loading_register(cpu_t *self, const byte_t new_reg_value);

// new
void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, addressing_mode_e mode,
                          const memory_t *memory);
void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        addressing_mode_e mode);
void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void cpu_and_with_accumulator(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
word_t cpu_resolve_first_operand(const cpu_t *self, const addressing_mode_e mode, bool *return_value_is_address);
void cpu_jump_subroutine(cpu_t *self, memory_t *memory);
void cpu_return_from_subroutine(cpu_t *self, memory_t *memory);
void cpu_branch_based_on_flag(cpu_t *self, const byte_t mask, const bool branch_if_set);
void cpu_compare(cpu_t *self, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode);
void cpu_decrement_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode);
void cpu_decrement_register(cpu_t *self, byte_t *register_ptr);
void cpu_test_bit(cpu_t *self, memory_t *memory, const addressing_mode_e mode);

#define MAKE_WORD(a, b) ((a << 8) | (b))

#define CARRY_MASK 0x1
#define ZERO_MASK 0x2
#define INTERRUPT_MASK 0x4
#define DECIMAL_MASK 0x8
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

      self->state = FETCH_OPERAND;
      cpu_set_remaining_bytes(self);

      break;
    case FETCH_OPERAND:
      if (self->remaining_bytes == 0) {
        self->state = DECODE;
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
      self->state = EXECUTE;
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
    case DEX_OPCOD:
    case DEY_OPCOD:
    case CLC_OPCOD:
    case CLD_OPCOD:
    case CLI_OPCOD:
    case CLV_OPCOD:
    case LDAI_OPCOD:
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
      bytes = 1;
      break;
    case DECA_OPCOD:
    case DECAX_OPCOD:
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
      bytes = 2;
      break;
    case NOP_OPCOD:
    case RTS_OPCOD:
      break;
  }

  self->remaining_bytes = bytes;
}

void cpu_exec(cpu_t *self, memory_t *memory) {
  switch ((opcode_e)self->reg_IR) {
    case NOP_OPCOD:
      puts("No operation;");
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
      cpu_jump(self);
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
    case BVC_OPCOD:
      cpu_branch_based_on_flag(self, OVERFLOW_MASK, false);
      break;
    case BVS_OPCOD:
      cpu_branch_based_on_flag(self, OVERFLOW_MASK, true);
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
    default:
      self->last_trap = ILLEGAL_OPCODE;
  }
}

void cpu_load_to_register(cpu_t *self, byte_t *register_ptr, char register_name, const addressing_mode_e mode,
                          const memory_t *memory) {
  puts("Load to register;");
  bool suc = true;
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(self, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, first_operand, &suc);
  } else {
    value = first_operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_flags_when_loading_register(self, value);
  *register_ptr = value;
}

void cpu_and_with_accumulator(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  puts("Logical AND with accumulator;");
  bool suc = true;
  bool is_address = true;

  const word_t operand = cpu_resolve_first_operand(self, mode, &is_address);

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

  cpu_update_flags_when_loading_register(self, self->reg_A);
}

void cpu_store_register(cpu_t *self, byte_t register_value, const char register_name, memory_t *memory,
                        const addressing_mode_e mode) {
  puts("Store register;");

  const word_t address = cpu_resolve_first_operand(self, mode, NULL);

  memory_write(memory, address, register_value);
}

void cpu_add_to_accumulator(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  puts("Add to accumulator;");
  bool suc = true;
  bool is_address = true;
  byte_t value = cpu_resolve_first_operand(self, mode, &is_address);

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
  cpu_update_flags_when_loading_register(self, self->reg_A);
}

word_t cpu_resolve_first_operand(const cpu_t *self, const addressing_mode_e mode, bool *return_value_is_address) {
  word_t value = 0;

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
      value = MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) & 0xFFFF;
      break;
    case ABSOLUTE_X:
      value = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_X) & 0xFFFF;
      break;
    case ABSOLUTE_Y:
      value = (MAKE_WORD(self->operands_buffer[1], self->operands_buffer[0]) + self->reg_Y) & 0xFFFF;
      break;
    default:
      return 0;
  }

  return value;
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

void cpu_push_value_onto_stack(cpu_t *self, memory_t *memory, const byte_t value) {
  puts("Pushing onto the stack;");

  if (self->reg_SP == 0) {
    puts("Error: stack overflow");
    self->last_trap = STACK_OVERFLOW;
    return;
  }

  memory_write(memory, STACK_LOWEST_ADDRESS + self->reg_SP--, value);
}

byte_t cpu_pull_from_stack(cpu_t *self, memory_t *memory) {
  puts("Pulling from the stack;");
  bool suc = true;

  if (self->reg_SP == 0xFF) {
    puts("Error: stack underflow");
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
  puts("Jumping to subroutine;");
  const word_t jumping_address = cpu_resolve_first_operand(self, ABSOLUTE, NULL);

  const word_t pushing_address =
      self->reg_IP - 1;  // address of the next instruction to execute after the subroutine call

  cpu_push_value_onto_stack(self, memory, (pushing_address >> 8) & 0xFF);  // high
  cpu_push_value_onto_stack(self, memory, pushing_address & 0xFF);         // low

  self->reg_IP = jumping_address;
}

void cpu_return_from_subroutine(cpu_t *self, memory_t *memory) {
  puts("Return from subroutine;");
  word_t address = cpu_pull_from_stack(self, memory);
  address += (cpu_pull_from_stack(self, memory) << 8);
  self->reg_IP = address + 1;
}

void cpu_jump(cpu_t *self) {
  puts("Jump;");
  const word_t address = cpu_resolve_first_operand(self, ABSOLUTE, NULL);

  self->reg_IP = address;
}

byte_t cpu_fetch(cpu_t *self, memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}

void cpu_branch_based_on_flag(cpu_t *self, const byte_t mask, const bool branch_if_set) {
  puts("Branching;");
  bool suc = true;

  const byte_t offset = cpu_resolve_first_operand(self, RELATIVE, NULL);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const bool flag_is_set = cpu_status_flag_is_set(self, mask);

  if ((branch_if_set && flag_is_set) || (!branch_if_set && !flag_is_set)) {
    self->reg_IP += (int8_t)offset;
  }
}

void cpu_compare(cpu_t *self, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode) {
  puts("Comparing;");
  bool suc = true;
  bool is_address = true;
  word_t value = cpu_resolve_first_operand(self, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t result = (byte_t)(register_value - (byte_t)value);
  cpu_update_flags_when_loading_register(self, result);

  if (register_value >= value) {
    cpu_status_flag_set(self, CARRY_MASK);
  } else {
    cpu_status_flag_clear(self, CARRY_MASK);
  }
}

#define DEC(x) ((x) - 1)

void cpu_decrement_memory(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  bool suc = true;

  const word_t address = cpu_resolve_first_operand(self, mode, NULL);

  const byte_t value = DEC(memory_read(memory, address, &suc));

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_flags_when_loading_register(self, value);

  memory_write(memory, address, value);
}

void cpu_decrement_register(cpu_t *self, byte_t *register_ptr) {
  --(*register_ptr);
  cpu_update_flags_when_loading_register(self, *register_ptr);
}

void cpu_test_bit(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  bool suc;

  const word_t address = cpu_resolve_first_operand(self, mode, NULL);

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

void cpu_dump_one_flag(const cpu_t *self, const char *flag_name, const byte_t mask, FILE *stream) {
  if (cpu_status_flag_is_set(self, mask)) {
    fprintf(stream, "\t%s flag is set\n", flag_name);
  } else {
    fprintf(stream, "\t%s flag is clear\n", flag_name);
  }
}

void cpu_dump_processor_status(const cpu_t *self, FILE *stream) {
  cpu_dump_one_flag(self, "Carry", CARRY_MASK, stream);
  cpu_dump_one_flag(self, "Zero", ZERO_MASK, stream);
  cpu_dump_one_flag(self, "Interrupt", INTERRUPT_MASK, stream);
  cpu_dump_one_flag(self, "Decimal", DECIMAL_MASK, stream);
  cpu_dump_one_flag(self, "Break", BREAK_MASK, stream);
  cpu_dump_one_flag(self, "Overflow", OVERFLOW_MASK, stream);
  cpu_dump_one_flag(self, "Negative", NEGATIVE_MASK, stream);
}

void cpu_dump(const cpu_t *self, FILE *stream) {
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
