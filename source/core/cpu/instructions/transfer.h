#pragma once

#include "instructions.h"

void transfer_a_to_x_instr(const instr_context_t *context);
void transfer_a_to_y_instr(const instr_context_t *context);
void transfer_x_to_a_instr(const instr_context_t *context);
void transfer_y_to_a_instr(const instr_context_t *context);
void transfer_sp_to_x_instr(const instr_context_t *context);
void transfer_x_to_sp_instr(const instr_context_t *context);
