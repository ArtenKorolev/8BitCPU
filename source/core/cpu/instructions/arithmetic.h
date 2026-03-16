#pragma once

#include "instructions_data.h"

void add_to_accumulator_instr(const instr_context_t *context);
void sub_from_accumulator_instr(const instr_context_t *context);

void dec_memory_instr(const instr_context_t *context);
void inc_memory_instr(const instr_context_t *context);

void inc_x_instr(const instr_context_t *context);
void inc_y_instr(const instr_context_t *context);
void dec_x_instr(const instr_context_t *context);
void dec_y_instr(const instr_context_t *context);
