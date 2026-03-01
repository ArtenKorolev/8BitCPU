#pragma once

#include "cpu.h"
#include "memory.h"

typedef struct {
  cpu_t cpu;
  memory_t memory;
  bool valid;
} emulator_t;

typedef enum {
  APPLE2,
  COMMONDORE64,  // not supported
  NES,           // not supported
} emulator_type_e;

void emulator_init(emulator_t *self, emulator_type_e type);
int emulator_run(emulator_t *self);
void emulator_shutdown(emulator_t *self);
