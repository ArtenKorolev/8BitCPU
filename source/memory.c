#include "memory.h"

#include <stdlib.h>
#include <string.h>

#include "base.h"

void memory_init(memory_t *self) {
  byte_t *new_memory = (byte_t *)malloc(1 * MEMORY_SIZE);  // NOLINT

  if (new_memory == NULL) {
    self->memory = NULL;
    self->memory_size = 0;
  } else {
    self->memory = new_memory;
    self->memory_size = MEMORY_SIZE;
  }
}

byte_t memory_read(memory_t *self, const word_t address, bool *success) {
  if (address > self->memory_size || address < 0) {
    *success = false;
    return 0;
  }

  *success = true;

  return self->memory[address];
}
