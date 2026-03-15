#include "branch.h"

#include "cpu.h"
#include "log.h"

void branch_on_flag_instr(cpu_t *self, byte_t flag, bool branch_if_set);

void bcc_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, CARRY, false);
}

void bcs_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, CARRY, true);
}

void beq_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, ZERO, true);
}

void bne_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, ZERO, false);
}

void bmi_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, NEGATIVE, true);
}

void bpl_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, NEGATIVE, false);
}

void bvs_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, OVERFLOW_, true);
}

void bvc_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, OVERFLOW_, false);
}

inline void branch_on_flag_instr(cpu_t *self, const byte_t flag, const bool branch_if_set) {
  emu_log(INFO, "Branching;\n");

  const byte_t offset = cpu_resolve_first_operand(self, NULL, RELATIVE, NULL);

  if (self->last_trap != OK) {
    return;
  }

  const bool flag_is_set = cpu_status_flag_is_set(self, flag);

  if ((branch_if_set && flag_is_set) || (!branch_if_set && !flag_is_set)) {
    emu_log(INFO, "Condition is satisfied, jump;\n");
    self->reg_IP += (int8_t)offset;
  } else {
    emu_log(INFO, "Condition is not satisfied;\n");
  }
}
