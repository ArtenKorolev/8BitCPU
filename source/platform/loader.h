#pragma once

#include <stdlib.h>
#include <string.h>

#include "apple2.h"
#include "emulator.h"
#include "log.h"
#include "platform_interface.h"

inline static void load_platform_interface(emulator_t *self) {
  platform_interface_t interface;
  memset(&interface, 0, sizeof(interface));  // set all pointers to NULL

  switch (self->type) {
    case APPLE2:
      interface.read = apple2_memory_read;
      interface.write = apple2_memory_write;
      interface.render = apple2_render;
      break;
    default:
      emu_log(ERROR, "Unsupported platform type\n");
      self->valid = false;
      return;
  }

  self->platform_interface = interface;

  self->valid = true;
}

inline static void load_computer_data(emulator_t *self) {
  apple2_data_t *ap_data = NULL;

  switch (self->type) {
    case APPLE2:
      ap_data = (apple2_data_t *)malloc(sizeof(apple2_data_t));

      if (ap_data == NULL) {
        emu_log(ERROR, "malloc failed\n");
        self->valid = false;
        return;
      }

      ap_data->term_render_request = true;
      self->memory.computer_data = (void *)ap_data;
      return;
    default:
      emu_log(ERROR, "Unsupported platform type\n");
      self->valid = false;
      return;
      break;
  }
}
