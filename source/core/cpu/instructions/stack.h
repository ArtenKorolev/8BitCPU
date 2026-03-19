#pragma once

#include "instructions_data.h"

void push_word_onto_stack(cpu_t *cpu, memory_t *memory, word_t word);
word_t pull_word_from_stack(cpu_t *cpu, memory_t *memory);

void push_onto_stack_instr(cpu_t *self, memory_t *memory, byte_t value);
byte_t pull_from_stack_instr(cpu_t *self, const memory_t *memory);

void jump_subroutine_instr(const instr_context_t *context);
void return_from_subroutine_instr(const instr_context_t *context);
void push_a_instr(const instr_context_t *context);
void push_processor_status_instr(const instr_context_t *context);
void pull_a_instr(const instr_context_t *context);
void pull_processor_status_instr(const instr_context_t *context);
