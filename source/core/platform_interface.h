#pragma once

#include "memory.h"

typedef void (*render_func_ptr_t)(const memory_t *emulator, void *data);

typedef struct {
  read_func_ptr_t read;
  write_func_ptr_t write;
  render_func_ptr_t render;
} platform_interface_t;
