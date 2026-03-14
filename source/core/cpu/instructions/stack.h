#pragma once

#include "instructions.h"

void jump_subroutine_instr(const instr_context_t *context);
void return_from_subroutine_instr(const instr_context_t *context);
void push_a_instr(const instr_context_t *context);
void push_processor_status_instr(const instr_context_t *context);
void pull_a_instr(const instr_context_t *context);
void pull_processor_status_instr(const instr_context_t *context);
