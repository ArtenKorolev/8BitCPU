#pragma once

#include "base.h"
#include "memory.h"

byte_t apple2_memory_read(const memory_t *self, word_t address, bool *success);
void apple2_memory_write(memory_t *self, word_t address, byte_t value);
