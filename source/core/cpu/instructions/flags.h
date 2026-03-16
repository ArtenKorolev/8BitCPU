#pragma once

#include "cpu.h"
#include "instructions_data.h"

static inline void clc_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, CARRY);
}

static inline void sec_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, CARRY);
}

static inline void cli_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, INTERRUPT);
}

static inline void sei_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, INTERRUPT);
}

static inline void clv_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, OVERFLOW_);
}

static inline void cld_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, DECIMAL);
}

static inline void sed_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, DECIMAL);
}
