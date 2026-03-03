#pragma once

#include "apple2.h"
#include "emulator.h"
#include "log.h"
#include "machine_interface.h"

inline void load_machine_interface(emulator_t *self) {
  machine_interface_t interface;
  memset(&interface, 0, sizeof(interface));  // set all pointers to NULL

  switch (self->type) {
    case APPLE2:
      interface.read = apple2_memory_read;
      interface.write = apple2_memory_write;
      interface.render = apple2_render;
      break;
    default:
      emu_log(ERROR, "Unsupported machine type\n");
      self->valid = false;
      return;
  }

  self->machine_interface = interface;

  self->valid = true;
}
