#pragma once

#include "memory.h"

typedef struct _emulator emulator_t;
typedef void (*render_func_ptr_t)(const memory_t *emulator);

typedef struct {
  read_func_ptr_t read;
  write_func_ptr_t write;
  render_func_ptr_t render;
} machine_interface_t;
