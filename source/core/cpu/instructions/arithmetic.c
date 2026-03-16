#include "arithmetic.h"

#include "cpu.h"
#include "instructions_data.h"
#include "log.h"

void add_to_accumulator_instr(const instr_context_t *context) {
  emu_log(INFO, "Add to accumulator;\n");

  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t carry = cpu_status_flag_is_set(context->cpu, CARRY) ? 1 : 0;
  const word_t result = value + context->cpu->reg_A + carry;

  cpu_update_overflow_flag_in_arithmetic(context->cpu, result, value);
  cpu_update_carry_flag(context->cpu, result);

  context->cpu->reg_A = (byte_t)result;
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void sub_from_accumulator_instr(const instr_context_t *context) {
  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t borrow = cpu_status_flag_is_set(context->cpu, CARRY) ? 0 : 1;
  const word_t result = (word_t)context->cpu->reg_A - (byte_t)value - borrow;

  cpu_update_overflow_flag_in_arithmetic(context->cpu, result, value);

  if (result < 0x100) {
    cpu_status_flag_set(context->cpu, CARRY);
  } else {
    cpu_status_flag_clear(context->cpu, CARRY);
  }

  context->cpu->reg_A = (byte_t)result;
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void change_by_one(const instr_context_t *context, const byte_t value_to_add);

void dec_memory_instr(const instr_context_t *context) {
  emu_log(INFO, "Decrement memory;\n");
  change_by_one(context, -1);
}

void inc_memory_instr(const instr_context_t *context) {
  emu_log(INFO, "Increment memory;\n");
  change_by_one(context, 1);
}

inline void change_by_one(const instr_context_t *context, const byte_t value_to_add) {
  const word_t address = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, NULL);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode) + value_to_add;
  cpu_update_zero_and_negative_flags(context->cpu, value);
  memory_write(context->memory, address, value);
}

void dec_register_instr(cpu_t *self, byte_t *register_ptr);
void inc_register_instr(cpu_t *self, byte_t *register_ptr);

void inc_x_instr(const instr_context_t *context) {
  inc_register_instr(context->cpu, &context->cpu->reg_X);
}

void inc_y_instr(const instr_context_t *context) {
  inc_register_instr(context->cpu, &context->cpu->reg_Y);
}

void dec_x_instr(const instr_context_t *context) {
  dec_register_instr(context->cpu, &context->cpu->reg_X);
}

void dec_y_instr(const instr_context_t *context) {
  dec_register_instr(context->cpu, &context->cpu->reg_Y);
}

inline void dec_register_instr(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Decrement register;\n");
  --(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}

inline void inc_register_instr(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Increment register;\n");
  ++(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}
