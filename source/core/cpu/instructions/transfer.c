#include "transfer.h"

#include "cpu.h"
#include "log.h"

void transfer_regs_instr(cpu_t *self, const byte_t *from, byte_t *to);

void transfer_a_to_x_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_A, &context->cpu->reg_X);
}

void transfer_a_to_y_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_A, &context->cpu->reg_Y);
}

void transfer_x_to_a_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_X, &context->cpu->reg_A);
}

void transfer_y_to_a_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_Y, &context->cpu->reg_A);
}

void transfer_sp_to_x_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_SP, &context->cpu->reg_X);
}

void transfer_x_to_sp_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_X, &context->cpu->reg_SP);
}

inline void transfer_regs_instr(cpu_t *self, const byte_t *from, byte_t *to) {
  emu_log(INFO, "Transferring registers;\n");

  if (from == NULL || to == NULL) {
    return;
  }

  *to = *from;
  cpu_update_zero_and_negative_flags(self, *to);
}
