#include "apple2.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

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

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

static byte_t keyboard_latch = 0;
static bool keyboard_ready = false;

#define KEYBOARD_READY_MASK 0x80
#define DISCARD_BIT7 0x7F

typedef enum {
  TEXT,
  GRAPHICS
} video_mode_e;

typedef enum {
  HI,
  LO
} res_e;

typedef enum {
  PRIM,
  SEC
} page_e;

static video_mode_e g_video_mode = TEXT;
static page_e g_page = SEC;
static res_e g_res = LO;

bool apple2_terminal_should_render = true;

word_t calculate_row_address(word_t base_address, byte_t row_counter);
void render(const memory_t *memory, page_e page);

byte_t apple2_memory_read(const memory_t *self, const word_t address, bool *success) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    *success = false;
    return 0;
  }

  *success = true;

  switch (address) {
    case SET_GRAPHICS:
      emu_log(INFO, "Set graphics;\n");
      g_video_mode = GRAPHICS;
      return 0;
    case SET_TEXT:
      emu_log(INFO, "Set text;\n");
      g_video_mode = TEXT;
      return 0;
    case SET_PRIM:
      emu_log(INFO, "Set primary page;\n");
      g_page = PRIM;
      return 0;
    case SET_SEC:
      emu_log(INFO, "Set secondary page;\n");
      g_page = SEC;
      return 0;
    case SET_LO:
      emu_log(INFO, "Set lo mode;\n");
      g_res = LO;
      return 0;
    case SET_HI:
      emu_log(INFO, "Set hi mode;\n");
      g_res = HI;
      return 0;
    case KEYBOARD_DATA:
      emu_log(INFO, "Reading keyboard;\n");

      if (!keyboard_ready) {
        int input_chr = getchar();

        input_chr = (input_chr == '\n' ? '\r' : input_chr);

        keyboard_latch = (byte_t)input_chr | KEYBOARD_READY_MASK;
        keyboard_ready = true;
      }

      return keyboard_latch;
    case KEYBOARD_STROBE:
      emu_log(INFO, "Keyboard strobe;\n");
      keyboard_ready = false;
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

void apple2_memory_write(memory_t *self, const word_t address, const byte_t value) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    return;
  }

  if ((address >= TEXT_START_PRIM && address <= TEXT_END_PRIM) ||
      (address >= TEXT_START_SEC && address <= TEXT_END_SEC)) {
    apple2_terminal_should_render = true;
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

void apple2_render(const memory_t *memory) {
  if (!apple2_terminal_should_render) {
    return;
  }

  render(memory, g_page);

  apple2_terminal_should_render = false;
}

#define COLS_COUNT 40
#define ROWS_COUNT 24

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

  for (int row = 0; row < ROWS_COUNT; row++) {
    const word_t row_addr = calculate_row_address(base, row);

    for (int col = 0; col < COLS_COUNT; col++) {
      byte_t symbol = memory_read(memory, row_addr + col, &suc);

      if (symbol == '\n') {
        symbol = '?';
      } else {
        symbol &= DISCARD_BIT7;

        if (symbol < 0x20) {
          symbol += 0x40;
        }

        if (!suc || !isprint(symbol)) {
          symbol = ' ';
        }
      }

      putchar(symbol);
    }

    puts("|");
  }

  for (int i = 0; i < COLS_COUNT; ++i) {
    putchar('=');
  }

  putchar('\n');

  g_log_level = saved;
}

inline word_t calculate_row_address(const word_t base_address, const byte_t row_counter) {
  return base_address + ((row_counter & 7) * 0x80) + ((row_counter >> 3) * 0x28);
}
