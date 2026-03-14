#pragma once

#include "cpu.h"
#include "memory.h"
#include "platform_interface.h"

typedef enum {
  APPLE2,
  COMMODORE64,  // not supported
  NES,          // not supported
} machine_type_e;

struct Emulator {
  cpu_t cpu;
  memory_t memory;

  bool valid;
  machine_type_e type;
  platform_interface_t machine_interface;
};

void emulator_init(emulator_t *self, machine_type_e type);
int emulator_run(emulator_t *self);
void emulator_shutdown(emulator_t *self);
