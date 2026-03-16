#include "memory.h"

#include <stdlib.h>

void memory_init(memory_t *self, const read_func_ptr_t read_func, const write_func_ptr_t write_func) {
  byte_t *new_memory = malloc(MEMORY_SIZE);

  if (new_memory == NULL) {
    self->memory = NULL;
    self->memory_size = 0;
  } else {
    self->memory = new_memory;
    self->memory_size = MEMORY_SIZE;
  }

  self->write = write_func;
  self->read = read_func;
}

void memory_write(memory_t *self, const word_t address, const byte_t value) {
  if (self == NULL || self->write == NULL) {
    return;
  }

  self->write(self, address, value, self->computer_data);
}

byte_t memory_read(const memory_t *self, const word_t address, bool *success) {
  if (self == NULL || self->read == NULL) {
    return 0;
  }

  return self->read(self, address, success, self->computer_data);
}

void memory_free(memory_t *self) {
  if (self == NULL || self->memory_size == 0 || self->memory == 0) {
    return;
  }

  free(self->memory);
  self->memory = NULL;
  self->memory_size = 0;
}
