#include "emulator.h"

#include <ctype.h>
#include <string.h>

#include "apple2.h"
#include "file_io.h"
#include "log.h"
#include "memory.h"

void emulator_read_file_into_memory(emulator_t *self, const char *file_name);

void emulator_init(emulator_t *self, const emulator_type_e type) {
  emu_log(INFO, "Start initializing emulator;\n");
  self->valid = false;

  read_func_ptr_t read = NULL;
  write_func_ptr_t write = NULL;

  switch (type) {
    case APPLE2:
      read = apple2_memory_read;
      write = apple2_memory_write;
      break;
    default:  // unsupported machine type
      emu_log(ERROR, "Unsupported machine type\n");
      emu_log(ERROR, "Emulator initialization failed;\n");
      return;
  }

  self->valid = true;

  memory_init(&self->memory, read, write);
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
    self->valid = false;
    file_content_free(&file_content);
    return;
  }

  if (file_content.size > MEMORY_SIZE) {
    memcpy(self->memory.memory, file_content.data, MEMORY_SIZE);
  } else {
    memcpy(self->memory.memory, file_content.data, file_content.size);
  }

  file_content_free(&file_content);
}

#define ASSERT_ON_VALID(self) \
  if (!self->valid) {         \
    return;                   \
  }

#define TEXT_FRAME_BUFFER_START 0x400
#define TEXT_FRAME_BUFFER_END 0x07FF

void emulator_render_text(const emulator_t *self) {
  bool suc;

  switch (g_log_level) {
    case NO_LOG:
    case ERROR:
      printf("\x1B[2J\x1B[1;1H");
    default:
      break;
  }

  puts("\n=== APPLE II terminal ===");

  for (int i = TEXT_FRAME_BUFFER_START; i <= TEXT_FRAME_BUFFER_END; ++i) {
    char symbol = memory_read(&self->memory, i, &suc);

    if (!suc) {
      symbol = ' ';
    }

    if (isprint(symbol)) {
      putchar(symbol);
    }

    if (i % 40 == 0) {
      putchar(0xa);  // new line every 40 characters
    }
  }

  puts("\n=== APPLE II terminal ===\n");
}

int emulator_run(emulator_t *self) {
  if (!self->valid) {
    return 1;
  }

  emu_log(INFO, "Emulation started;\n");

  size_t cycles = 0;

  while (self->valid) {
    const trap_e cycle_result = cpu_do_cycle(&self->cpu, &self->memory);

    if (cycle_result != OK) {
      self->valid = false;
      emu_log(ERROR, "!TRAP!\nTrap number: %d\n", cycle_result);
      break;
    }

    // for (int i = 0; i < 100000000; ++i);

    ++cycles;

    if (cycles > 10000) {
      emulator_render_text(self);
      cycles = 0;
    }
  }

  emu_log(INFO, "Emulation ended;\n");

  return (int)!self->valid;
}

void emulator_shutdown(emulator_t *self) {
  ASSERT_ON_VALID(self)
  memory_free(&self->memory);
}
