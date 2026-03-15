#include "emulator.h"

#include <ctype.h>
#include <string.h>

#include "base.h"
#include "file_io.h"
#include "loader.h"
#include "log.h"
#include "memory.h"

inline int min(const int first, const int second) {
  return (first < second ? first : second);
}

void emulator_read_file_into_memory(emulator_t *self, const char *file_name);

void emulator_init(emulator_t *self, const machine_type_e type) {
  emu_log(INFO, "Start initializing emulator;\n");
  self->valid = false;
  self->type = type;

  load_machine_interface(self);

  if (!self->valid) {
    emu_log(ERROR, "Emulator initialization failed;\n");
    return;
  }

  memory_init(&self->memory, self->machine_interface.read, self->machine_interface.write);
  emulator_read_file_into_memory(self, "mem.bin");

  if (!self->valid) {
    emu_log(ERROR, "Emulator initialization failed;\n");
    return;
  }

  cpu_init(&self->cpu, &self->memory);

  self->valid = true;
  emu_log(INFO, "Emulator initialized successfully;\n");
}

void emulator_read_file_into_memory(emulator_t *self, const char *file_name) {
  file_content_t file_content = read_bin_file(file_name);

  if (file_content.size == 0) {
    emu_log(ERROR, "Zero bytes of program are read\n");
    emu_log(WARN, "Note that ROM file \"mem.bin\" should be 64 KB\n");
    self->valid = false;
    file_content_free(&file_content);
    return;
  }

  if (file_content.size != KB_64) {
    emu_log(WARN, "Note that ROM file \"mem.bin\" should be 64 KB (got %d bytes)\n", file_content.size);
  }

  memcpy(self->memory.memory, file_content.data, min(MEMORY_SIZE, file_content.size));

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

    self->machine_interface.render(&self->memory);
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
