#include "memory.h"

void memory_init(memory_t *self, read_func_ptr_t read_func, write_func_ptr_t write_func) {
  byte_t *new_memory = (byte_t *)malloc(MEMORY_SIZE);

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
  self->write(self, address, value);
}

byte_t memory_read(const memory_t *self, const word_t address, bool *success) {
  return self->read(self, address, success);
}

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

void memory_free(memory_t *self) {
  if (IS_INVALID_MEMORY(self)) {
    return;
  }

  free(self->memory);
  self->memory = NULL;
  self->memory_size = 0;
}
