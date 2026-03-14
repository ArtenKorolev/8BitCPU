#include "stack.h"

#include "cpu.h"
#include "log.h"

void push_onto_stack_instr(cpu_t *self, memory_t *memory, byte_t value);
byte_t pull_from_stack_instr(cpu_t *self, const memory_t *memory);

void push_a_instr(const instr_context_t *context) {
  push_onto_stack_instr(context->cpu, context->memory, context->cpu->reg_A);
}

void push_processor_status_instr(const instr_context_t *context) {
  push_onto_stack_instr(context->cpu, context->memory, context->cpu->reg_P);
}

void pull_a_instr(const instr_context_t *context) {
  context->cpu->reg_A = pull_from_stack_instr(context->cpu, context->memory);
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void pull_processor_status_instr(const instr_context_t *context) {
  context->cpu->reg_P = pull_from_stack_instr(context->cpu, context->memory);
}

#define STACK_LOWEST_ADDRESS 0x0100
#define FULL_STACK_PTR 0
#define EMPTY_STACK_PTR 0xFF

inline void push_onto_stack_instr(cpu_t *self, memory_t *memory, const byte_t value) {
  emu_log(INFO, "Pushing onto the stack;\n");

  if (self->reg_SP == FULL_STACK_PTR) {
    emu_log(ERROR, "Stack overflow;\n");
    self->last_trap = STACK_OVERFLOW;
    return;
  }

  memory_write(memory, STACK_LOWEST_ADDRESS + self->reg_SP--, value);
}

inline byte_t pull_from_stack_instr(cpu_t *self, const memory_t *memory) {
  emu_log(INFO, "Pulling from the stack;\n");

  bool suc = true;

  if (self->reg_SP == EMPTY_STACK_PTR) {
    emu_log(ERROR, "Stack underflow;\n");
    self->last_trap = STACK_UNDERFLOW;
    return 0;
  }

  const byte_t value = memory_read(memory, STACK_LOWEST_ADDRESS + ++self->reg_SP, &suc);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return 0;
  }

  return value;
}

void jump_subroutine_instr(const instr_context_t *context) {
  emu_log(INFO, "Jumping to subroutine;\n");

  const word_t jumping_address = cpu_resolve_first_operand(context->cpu, context->memory, ABSOLUTE, NULL);

  emu_log(INFO, "Jumping address: %x;\n", jumping_address);

  const word_t pushing_address =
      context->cpu->reg_IP - 1;  // address of the next instruction to execute after the subroutine call

  push_onto_stack_instr(context->cpu, context->memory, (pushing_address >> 8) & 0xFF);  // high
  push_onto_stack_instr(context->cpu, context->memory, pushing_address & 0xFF);         // low

  context->cpu->reg_IP = jumping_address;
}

word_t fetch_return_address(cpu_t *cpu, memory_t *memory);

void return_from_subroutine_instr(const instr_context_t *context) {
  emu_log(INFO, "Return from subroutine;\n");

  const word_t return_address = fetch_return_address(context->cpu, context->memory) + 1;

  emu_log(INFO, "Return address: %x;\n", return_address);

  context->cpu->reg_IP = return_address;
}

inline word_t fetch_return_address(cpu_t *cpu, memory_t *memory) {
  word_t return_address = pull_from_stack_instr(cpu, memory);
  return_address += (pull_from_stack_instr(cpu, memory) << 8);
  return return_address;
}
