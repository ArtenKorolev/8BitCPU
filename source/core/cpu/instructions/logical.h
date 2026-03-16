#pragma once

#include "instructions_data.h"

void or_with_accumulator_instr(const instr_context_t *context);
void exclusive_or_instr(const instr_context_t *context);
void and_with_accumulator_instr(const instr_context_t *context);
void test_bit_instr(const instr_context_t *context);
