#include "shifts.h"

#include "cpu.h"
#include "log.h"
#include "memory.h"

void logical_shift_right_instr(const instr_context_t *context) {
  emu_log(INFO, "Logical shift right;\n");

  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, &is_address);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (value & 0x01) {
    cpu_status_flag_set(context->cpu, CARRY);
  } else {
    cpu_status_flag_clear(context->cpu, CARRY);
  }

  const byte_t result = value >> 1;
  cpu_update_zero_and_negative_flags(context->cpu, result);

  if (is_address) {
    memory_write(context->memory, first_operand, result);
  } else {
    context->cpu->reg_A = result;
  }
}

void arithmetic_shift_left(const instr_context_t *context) {
  emu_log(INFO, "Arithmetic shift left;\n");
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, &is_address);

  if (context->cpu->last_trap != OK) {
    return;
  }

  byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (value & 0x80) {
    cpu_status_flag_set(context->cpu, CARRY);
  } else {
    cpu_status_flag_clear(context->cpu, CARRY);
  }

  const byte_t result = value << 1;
  cpu_update_zero_and_negative_flags(context->cpu, result);

  if (is_address) {
    memory_write(context->memory, first_operand, result);
  } else {
    context->cpu->reg_A = result;
  }
}

void rotate_left(const instr_context_t *context) {
  emu_log(INFO, "Rotate left;\n");
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, &is_address);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  const bool saved_carry = cpu_status_flag_is_set(context->cpu, CARRY);

  if (value & 0x80) {
    cpu_status_flag_set(context->cpu, CARRY);
  } else {
    cpu_status_flag_clear(context->cpu, CARRY);
  }

  byte_t result = value << 1;
  result |= (saved_carry ? 0x1 : 0);
  cpu_update_zero_and_negative_flags(context->cpu, result);

  if (is_address) {
    memory_write(context->memory, first_operand, result);
  } else {
    context->cpu->reg_A = result;
  }
}

void rotate_right(const instr_context_t *context) {
  emu_log(INFO, "Rotate right;\n");
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, &is_address);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  const bool saved_carry = cpu_status_flag_is_set(context->cpu, CARRY);

  if (value & 0x01) {
    cpu_status_flag_set(context->cpu, CARRY);
  } else {
    cpu_status_flag_clear(context->cpu, CARRY);
  }

  byte_t result = value >> 1;
  result |= (saved_carry ? 0x80 : 0);
  cpu_update_zero_and_negative_flags(context->cpu, result);

  if (is_address) {
    memory_write(context->memory, first_operand, result);
  } else {
    context->cpu->reg_A = result;
  }
}
