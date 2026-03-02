#include "apple2.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

#define TEXT_FRAME_BUFFER_START 0x400
#define TEXT_FRAME_BUFFER_END 0x07FF

#define IO_START 0xC000
#define IO_END 0xCFFF

#define OUTPUT_PORT 0xC000  // memory mapped output
#define INPUT_PORT 0xC001   // memory mapped input

#define ROM_START 0xD000

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

bool apple2_terminal_should_render = true;

byte_t apple2_memory_read(const memory_t *self, const word_t address, bool *success) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    *success = false;
    return 0;
  }

  *success = true;

  if (address == INPUT_PORT) {
    emu_log(INFO, "Reading INPUT_PORT (keyboard), calling getchar()");

    int c = getchar();

    while (c == '\n') {
      c = getchar();
    }

    return c;
  }

  if (address >= ROM_START) {
    emu_log(INFO, "Reading from ROM at address %04X\n", address);
    return self->memory[address];
  }

  if (address < IO_START) {
    emu_log(INFO, "Reading RAM at address %04X\n", address);
    return self->memory[address];
  }

  emu_log(WARN, "Reading unhandled I/O address %04X\n", address);
  return self->memory[address];
}

void apple2_memory_write(memory_t *self, const word_t address, const byte_t value) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    return;
  }

  if (address >= TEXT_FRAME_BUFFER_START && address <= TEXT_FRAME_BUFFER_END) {
    apple2_terminal_should_render = true;
  }

  if (address == OUTPUT_PORT) {
    emu_log(INFO, "Writing OUTPUT_PORT (console), value %02X\n", value);
    putchar(value);
    fflush(stdout);
    return;
  }

  if (address >= ROM_START) {
    emu_log(WARN, "Attempted write to ROM at address %04X ignored\n", address);
    return;
  }

  if (address < IO_START) {
    emu_log(INFO, "Writing RAM at address %04X, value %02X\n", address, value);
    self->memory[address] = value;
    return;
  }

  emu_log(WARN, "Writing unhandled I/O address %04X, value %02X\n", address, value);
}

void apple2_render(const memory_t *memory) {
  if (!apple2_terminal_should_render) {
    return;
  }

  bool suc;

  printf("\x1B[2J\x1B[1;1H");

  const log_level_e saved = g_log_level;
  g_log_level = NO_LOG;

  puts("\nAPPLE II terminal\n");

  for (int i = 0; i < 40; ++i) {
    putchar('=');
  }

  putchar(0xa);

  int count = 0;

  for (int i = TEXT_FRAME_BUFFER_START; i <= TEXT_FRAME_BUFFER_END; ++i) {
    char symbol = memory_read(memory, i, &suc);

    if (!suc || !isprint(symbol)) {
      symbol = ' ';
    }

    putchar(symbol);
    ++count;

    if (count == 1024) {
      while (count % 40 != 0) {
        putchar(' ');
        ++count;
      }
    }

    if (count % 40 == 0) {
      puts("|");
    }
  }

  for (int i = 0; i < 40; ++i) {
    putchar('=');
  }

  putchar(0xa);

  g_log_level = saved;
  apple2_terminal_should_render = false;
}
