#include "interrupts.h"

#include "cpu.h"
#include "memory.h"
#include "stack.h"

enum {
  IRQ_VECTOR_LOW = 0xFFFE,
  IRQ_VECTOR_HIGH = 0xFFFF
};

inline word_t read_irq_vector(const memory_t *memory) {
  bool suc = true;
  word_t irq_vector = 0;

  irq_vector = memory_read(memory, IRQ_VECTOR_LOW, &suc);

  if (!suc) {
    return 0;
  }

  irq_vector |= (word_t)memory_read(memory, IRQ_VECTOR_HIGH, &suc) << 8;

  if (!suc) {
    return 0;
  }

  return irq_vector;
}

void force_interrupt_instr(const instr_context_t *context) {
  push_word_onto_stack(context->cpu, context->memory, context->cpu->reg_IP);
  push_onto_stack_instr(context->cpu, context->memory, context->cpu->reg_P);

  context->cpu->reg_IP = read_irq_vector(context->memory);

  cpu_status_flag_set(context->cpu, BREAK);
}

void return_from_interrupt(const instr_context_t *context) {
  context->cpu->reg_P = pull_from_stack_instr(context->cpu, context->memory);
  context->cpu->reg_IP = pull_word_from_stack(context->cpu, context->memory);
}
