#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base.h"
#include "log.h"

void memory_init(memory_t *self) {
  byte_t *new_memory = (byte_t *)malloc(MEMORY_SIZE);

  if (new_memory == NULL) {
    self->memory = NULL;
    self->memory_size = 0;
  } else {
    self->memory = new_memory;
    self->memory_size = MEMORY_SIZE;
  }
}

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

byte_t memory_read(const memory_t *self, const word_t address, bool *success) {
  if ((address > self->memory_size || address < 0) || IS_INVALID_MEMORY(self)) {
    *success = false;
    return 0;
  }

  *success = true;

  if (address == INPUT_PORT) {
    emu_log(INFO, "Trying to read an input port, call getchar();");
    return getchar();
  }

  emu_log(INFO, "Memory read at address %x\n", address);

  return self->memory[address];
}

void memory_write(memory_t *self, const word_t address, const byte_t value) {
  if (IS_INVALID_MEMORY(self)) {
    return;
  }

  if (address == OUTPUT_PORT) {  // memory mapped port
    emu_log(INFO, "Trying to write to output port, call putchar();");
    putchar(value);
    fflush(stdout);
    return;
  }

  emu_log(INFO, "Memory write at address %x, value %dx\n", address, value);

  self->memory[address] = value;
}

void memory_free(memory_t *self) {
  if (IS_INVALID_MEMORY(self)) {
    return;
  }

  free(self->memory);
}
