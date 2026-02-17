#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base.h"

typedef struct {
  byte_t *memory;
  size_t memory_size;
} memory_t;

void memory_init(memory_t *self);
void memory_free(memory_t *self);
byte_t memory_read(memory_t *self, word_t address, bool *success);
void memory_write(memory_t *self, word_t address, byte_t value);
