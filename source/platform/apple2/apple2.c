#include "apple2.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "opcodes.h"

enum {
  TEXT_START_PRIM = 0x0400,
  TEXT_END_PRIM = 0x07FF
};

enum {
  TEXT_START_SEC = 0x0800,
  TEXT_END_SEC = 0x0BFF
};

enum {
  IO_START = 0xC000,
  IO_END = 0xC0FF
};

enum {
  ROM_START = 0xD000
};

enum {
  KEYBOARD_DATA = 0xC000,
  KEYBOARD_STROBE = 0xC010,
  SPEAKER_TOGGLE = 0xC030
};

enum {
  SET_GRAPHICS = 0xC050,
  SET_TEXT = 0xC051,
  SET_PRIM = 0xC054,
  SET_SEC = 0xC055,
  SET_LO = 0xC056,
  SET_HI = 0xC057
};

#define KEYBOARD_READY_MASK 0x80
#define DISCARD_BIT7_MASK 0x7F

word_t calculate_row_address(word_t base_address, byte_t row_counter);
void render(const memory_t *memory, page_e page);

byte_t apple2_memory_read(const memory_t *self, const word_t address, bool *success, void *data) {
  if (self == NULL || self->memory_size == 0 || self->memory == NULL || address >= self->memory_size) {
    *success = false;
    return 0;
  }

  *success = true;

  apple2_data_t *ap_data = (apple2_data_t *)data;

  switch (address) {
    case SET_GRAPHICS:
      emu_log(INFO, "Set graphics;\n");
      ap_data->video_mode = GRAPHICS;
      return 0;
    case SET_TEXT:
      emu_log(INFO, "Set text;\n");
      ap_data->video_mode = TEXT;
      return 0;
    case SET_PRIM:
      emu_log(INFO, "Set primary page;\n");
      ap_data->page = PRIM;
      return 0;
    case SET_SEC:
      emu_log(INFO, "Set secondary page;\n");
      ap_data->page = SEC;
      return 0;
    case SET_LO:
      emu_log(INFO, "Set lo mode;\n");
      ap_data->res = LO;
      return 0;
    case SET_HI:
      emu_log(INFO, "Set hi mode;\n");
      ap_data->res = HI;
      return 0;
    case KEYBOARD_DATA:
      emu_log(INFO, "Reading keyboard;\n");

      if (!ap_data->keyboard_ready) {
        int input_chr = getchar();

        input_chr = (input_chr == '\n' ? '\r' : input_chr);

        ap_data->keyboard_latch = (byte_t)input_chr | KEYBOARD_READY_MASK;
        ap_data->keyboard_ready = true;
      }

      return ap_data->keyboard_latch;
    case KEYBOARD_STROBE:
      emu_log(INFO, "Keyboard strobe;\n");
      ap_data->keyboard_ready = false;
      return 0;
    case SPEAKER_TOGGLE:
      emu_log(INFO, "Speaker toggle;\n");
      return 0;
    default:
      break;
  };

  if (address >= ROM_START) {
    return self->memory[address];
  }

  if (address < IO_START) {
    return self->memory[address];
  }

  emu_log(WARN, "Unhandled I/O read %04X\n", address);
  return 0;
}

void apple2_memory_write(memory_t *self, const word_t address, const byte_t value, void *data) {
  if (self == NULL || self->memory_size == 0 || self->memory == NULL || address >= self->memory_size) {
    return;
  }

  apple2_data_t *ap_data = (apple2_data_t *)data;

  if ((address >= TEXT_START_PRIM && address <= TEXT_END_PRIM) ||
      (address >= TEXT_START_SEC && address <= TEXT_END_SEC)) {
    ap_data->term_render_request = true;
  }

  if (address >= ROM_START) {
    emu_log(WARN, "Write to ROM %04X ignored\n", address);
    return;
  }

  if (address < IO_START) {
    self->memory[address] = value;
    return;
  }

  emu_log(WARN, "Unhandled I/O write %04X value %02X\n", address, value);
}

void apple2_render(const memory_t *memory, void *data) {
  apple2_data_t *ap_data = (apple2_data_t *)data;

  if (!ap_data->term_render_request) {
    return;
  }

  render(memory, ap_data->page);

  ap_data->term_render_request = false;
}

enum {
  COLS_COUNT = 40,
  ROWS_COUNT = 24
};

void render(const memory_t *memory, const page_e page) {
  bool suc = true;

  printf("\x1B[2J\x1B[1;1H");

  const log_level_e saved = g_log_level;
  g_log_level = NO_LOG;

  puts("\nAPPLE II terminal\n");

  for (int i = 0; i < COLS_COUNT; ++i) {
    putchar('=');
  }

  putchar('\n');

  const word_t base = (page == PRIM) ? TEXT_START_PRIM : TEXT_START_SEC;

  static char screen[ROWS_COUNT * (COLS_COUNT + 2)] = {0};

  for (int row = 0; row < ROWS_COUNT; row++) {
    const word_t row_addr = calculate_row_address(base, row);

    for (int col = 0; col < COLS_COUNT; col++) {
      byte_t symbol = memory_read(memory, row_addr + col, &suc);

      if (symbol == NOP_OPCOD) {
        symbol = '?';
      } else {
        symbol &= DISCARD_BIT7_MASK;

        if (symbol < 0x20) {
          symbol += 0x40;
        }

        if (!suc || !isprint(symbol)) {
          symbol = ' ';
        }
      }

      screen[row * (COLS_COUNT + 2) + col] = symbol;
    }

    screen[row * (COLS_COUNT + 2) + (COLS_COUNT)] = '|';
    screen[row * (COLS_COUNT + 2) + (COLS_COUNT + 1)] = '\n';
  }

  write(STDOUT_FILENO, screen, ROWS_COUNT * (COLS_COUNT + 2));

  for (int i = 0; i < COLS_COUNT; ++i) {
    putchar('=');
  }

  putchar('\n');

  g_log_level = saved;
}

inline word_t calculate_row_address(const word_t base_address, const byte_t row_counter) {
  return base_address + ((row_counter & 7) * 0x80) + ((row_counter >> 3) * 0x28);
}
