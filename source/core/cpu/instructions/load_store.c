#include "load_store.h"

#include "cpu.h"
#include "log.h"
#include "memory.h"

void store_register_instr(const cpu_t *self, byte_t register_value, memory_t *memory, addressing_mode_e mode);
void load_register_instr(cpu_t *cpu, byte_t *register_ptr, addressing_mode_e mode, const memory_t *memory);

void load_a_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_A, context->mode, context->memory);
}

void load_x_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_X, context->mode, context->memory);
}

void load_y_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_Y, context->mode, context->memory);
}

void store_a_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_A, context->memory, context->mode);
}

void store_x_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_X, context->memory, context->mode);
}

void store_y_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_Y, context->memory, context->mode);
}

inline void store_register_instr(const cpu_t *self, const byte_t register_value, memory_t *memory,
                                 const addressing_mode_e mode) {
  emu_log(INFO, "Store register;\n");
  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);
  memory_write(memory, address, register_value);
}

inline void load_register_instr(cpu_t *cpu, byte_t *register_ptr, const addressing_mode_e mode,
                                const memory_t *memory) {
  emu_log(INFO, "Load to register;\n");
  const byte_t value = cpu_real_operand(cpu, memory, mode);
  cpu_update_zero_and_negative_flags(cpu, value);
  *register_ptr = value;
}
