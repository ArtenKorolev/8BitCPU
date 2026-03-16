#include "logical.h"

#include "cpu.h"
#include "log.h"

void exclusive_or_instr(const instr_context_t *context) {
  emu_log(INFO, "Exclusive OR;\n");
  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  context->cpu->reg_A ^= value;
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void and_with_accumulator_instr(const instr_context_t *context) {
  emu_log(INFO, "Logical AND with accumulator;\n");
  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  context->cpu->reg_A &= value;
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void or_with_accumulator_instr(const instr_context_t *context) {
  emu_log(INFO, "Logical OR with accumulator;\n");
  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  context->cpu->reg_A |= value;
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void test_bit_instr(const instr_context_t *context) {
  emu_log(INFO, "Test bit;\n");
  const byte_t value = cpu_real_operand(context->cpu, context->memory, context->mode);

  if (context->cpu->last_trap != OK) {
    return;
  }

  const byte_t result = context->cpu->reg_A & value;

  cpu_update_zero_flag(context->cpu, result);
  cpu_update_negative_flag(context->cpu, value);

  if (value & 0x40) {
    cpu_status_flag_set(context->cpu, OVERFLOW_);
  } else {
    cpu_status_flag_clear(context->cpu, OVERFLOW_);
  }
}
