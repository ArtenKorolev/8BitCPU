#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "base.h"

struct Memory;

typedef byte_t (*read_func_ptr_t)(const struct Memory *, const word_t, bool *);
typedef void (*write_func_ptr_t)(struct Memory *, const word_t, const byte_t);

struct Memory {
  byte_t *memory;
  size_t memory_size;

  read_func_ptr_t read;
  write_func_ptr_t write;
};

typedef struct Memory memory_t;

void memory_init(memory_t *self, read_func_ptr_t read_func, write_func_ptr_t write_func);
void memory_free(memory_t *self);

void memory_write(memory_t *self, word_t address, byte_t value);
byte_t memory_read(const memory_t *self, word_t address, bool *success);
