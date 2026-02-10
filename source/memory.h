#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base.h"

typedef struct {
  byte_t *memory;
  size_t memory_size;
} memory_t;

void memory_init(memory_t *self);
byte_t memory_read(memory_t *self, word_t index, bool *success);
