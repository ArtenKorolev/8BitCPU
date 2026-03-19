#pragma once

#include "instructions_data.h"

void force_interrupt_instr(const instr_context_t *context);
void return_from_interrupt(const instr_context_t *context);
