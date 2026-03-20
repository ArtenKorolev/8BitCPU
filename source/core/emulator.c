#include "emulator.h"

#include <ctype.h>
#include <string.h>

#include "base.h"
#include "file_io.h"
#include "loader.h"
#include "log.h"
#include "memory.h"
#include "options.h"

inline int min(const int first, const int second) {
  return (first < second ? first : second);
}

void emulator_read_rom_image_into_memory(emulator_t *self, const char *rom_file, word_t origin);

void emulator_init(emulator_t *self, const machine_type_e type, const options_t *options) {
  emu_log(INFO, "Start initializing emulator;\n");
  self->valid = false;
  self->type = type;

  load_platform_interface(self);

  if (!self->valid) {
    emu_log(ERROR, "Emulator initialization failed;\n");
    return;
  }

  memory_init(&self->memory, self->platform_interface.read, self->platform_interface.write);

  load_computer_data(self);

  emulator_read_rom_image_into_memory(self, options->rom_file, options->origin);

  if (!self->valid) {
    emu_log(ERROR, "Emulator initialization failed;\n");
    return;
  }

  cpu_init(&self->cpu, &self->memory);

  self->valid = true;
  emu_log(INFO, "Emulator initialized successfully;\n");
}

void emulator_read_rom_image_into_memory(emulator_t *self, const char *rom_file, const word_t origin) {
  if (rom_file == NULL) {
    emu_log(ERROR, "ROM image was not provided; \n");
    self->valid = false;
    return;
  }

  file_content_t file_content = read_bin_file(rom_file);

  emu_log(INFO, "ROM image: %s;\n", rom_file);
  emu_log(INFO, "Origin: %d;\n", origin);

  if (file_content.size == 0) {
    emu_log(ERROR, "Zero bytes of program are read;\n");
    self->valid = false;
    file_content_free(&file_content);
    return;
  }

  emu_log(INFO, "ROM size: %d bytes;\n", file_content.size);

  if (file_content.size + origin > KB_64) {
    emu_log(ERROR, "ROM image cannot fit in 64 KB;\n");
    self->valid = false;
    return;
  }

  memcpy(self->memory.memory + origin, file_content.data, file_content.size);

  file_content_free(&file_content);
}

int emulator_run(emulator_t *self) {
  if (!self->valid) {
    return 1;
  }

  emu_log(INFO, "Emulation started;\n");

  while (self->valid) {
    const trap_e cycle_result = cpu_do_cycle(&self->cpu, &self->memory);

    if (cycle_result != OK) {
      self->valid = false;
      emu_log(ERROR, "!TRAP!\nTrap number: %d\n", cycle_result);
      break;
    }

    self->platform_interface.render(&self->memory, self->memory.computer_data);
  }

  emu_log(INFO, "Emulation ended;\n");

  return (self->valid ? 0 : 1);
}

void emulator_shutdown(emulator_t *self) {
  if (!self->valid) {
    return;
  }

  memory_free(&self->memory);
}
