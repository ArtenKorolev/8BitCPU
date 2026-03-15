#include "compare.h"

#include "cpu.h"
#include "log.h"

void compare_instr(cpu_t *cpu, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode);

void cmp_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_A, context->mode);
}

void cpx_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_X, context->mode);
}

void cpy_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_Y, context->mode);
}

void compare_instr(cpu_t *cpu, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode) {
  emu_log(INFO, "Comparing;\n");

  const byte_t value = cpu_real_operand(cpu, memory, mode);

  if (cpu->last_trap != OK) {
    return;
  }

  const byte_t result = (byte_t)(register_value - (byte_t)value);
  cpu_update_zero_and_negative_flags(cpu, result);

  if (register_value >= (byte_t)value) {
    cpu_status_flag_set(cpu, CARRY);
  } else {
    cpu_status_flag_clear(cpu, CARRY);
  }
}
