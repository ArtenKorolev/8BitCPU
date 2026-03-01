#include "apple2.h"

#include <stdbool.h>
#include <stdio.h>

#include "log.h"

#define IO_START 0xC000
#define IO_END 0xCFFF

#define OUTPUT_PORT 0xC000  // memory mapped output
#define INPUT_PORT 0xC001   // memory mapped input

#define ROM_START 0xD000

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

byte_t apple2_memory_read(const memory_t *self, const word_t address, bool *success) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    *success = false;
    return 0;
  }

  *success = true;

  if (address == INPUT_PORT) {
    emu_log(INFO, "Reading INPUT_PORT (keyboard), calling getchar()");
    return getchar();
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
