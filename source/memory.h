#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base.h"

typedef struct {
  byte_t *memory;
  size_t memory_size;
} memory_t;

#define OUTPUT_PORT 0xFF00
#define INPUT_PORT 0xFF01

void memory_init(memory_t *self);
void memory_free(memory_t *self);
byte_t memory_read(const memory_t *self, word_t address, bool *success);
void memory_write(memory_t *self, word_t address, byte_t value);
