#pragma once

#include "instructions.h"

void bcc_instr(const instr_context_t *context);
void bcs_instr(const instr_context_t *context);
void beq_instr(const instr_context_t *context);
void bne_instr(const instr_context_t *context);
void bmi_instr(const instr_context_t *context);
void bpl_instr(const instr_context_t *context);
void bvs_instr(const instr_context_t *context);
void bvc_instr(const instr_context_t *context);
