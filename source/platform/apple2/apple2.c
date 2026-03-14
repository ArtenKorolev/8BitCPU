#include "apple2.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

#define TEXT_START_PRIM 0x0400
#define TEXT_END_PRIM 0x07FF

#define TEXT_START_SEC 0x0800
#define TEXT_END_SEC 0x0BFF

#define IO_START 0xC000
#define IO_END 0xC0FF

#define KEYBOARD_DATA 0xC000
#define KEYBOARD_STROBE 0xC010
#define SPEAKER_TOGGLE 0xC030

#define ROM_START 0xD000

#define IS_INVALID_MEMORY(self) (self->memory == NULL || self->memory_size == 0)

static byte_t keyboard_latch = 0;
static bool keyboard_ready = false;

typedef enum { TEXT, GRAPHICS } video_mode_e;
typedef enum { HI, LO } res_e;
typedef enum { PRIM, SEC } page_e;

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
    case 0xC050:
      emu_log(INFO, "Set graphics; ");
      g_video_mode = GRAPHICS;
      return 0;
    case 0xC051:
      emu_log(INFO, "Set text; ");
      g_video_mode = TEXT;
      return 0;
    case 0xC054:
      emu_log(INFO, "Set primary page; ");
      g_page = PRIM;
      return 0;
    case 0xC055:
      emu_log(INFO, "Set secondary page; ");
      g_page = SEC;
      return 0;
    case 0xC056:
      emu_log(INFO, "Set lo mode; ");
      g_res = LO;
      return 0;
    case 0xC057:
      emu_log(INFO, "Set hi mode; ");
      g_res = HI;
      return 0;
    case KEYBOARD_DATA:
      emu_log(INFO, "Reading keyboard; ");

      if (!keyboard_ready) {
        int c = getchar();

        c = (c == 0xA ? '\r' : c);

        keyboard_latch = (byte_t)c | 0x80;
        keyboard_ready = true;
      }

      return keyboard_latch;
    case KEYBOARD_STROBE:
      emu_log(INFO, "Keyboard strobe; ");
      keyboard_ready = false;
      return 0;
    case SPEAKER_TOGGLE:
      emu_log(INFO, "Speaker toggle; ");
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

void render(const memory_t *memory, const page_e page) {
  bool suc;

  printf("\x1B[2J\x1B[1;1H");

  const log_level_e saved = g_log_level;
  g_log_level = NO_LOG;

  puts("\nAPPLE II terminal\n");

  for (int i = 0; i < 40; ++i) {
    putchar('=');
  }

  putchar('\n');

  const word_t base = (page == PRIM) ? TEXT_START_PRIM : TEXT_START_SEC;

  for (int row = 0; row < 24; row++) {
    const word_t row_addr = calculate_row_address(base, row);

    for (int col = 0; col < 40; col++) {
      byte_t symbol = memory_read(memory, row_addr + col, &suc);

      if (symbol == 0xEA) {
        symbol = '?';
      } else {
        symbol &= 0x7F;

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

  for (int i = 0; i < 40; ++i) {
    putchar('=');
  }

  putchar('\n');

  g_log_level = saved;
}

inline word_t calculate_row_address(const word_t base_address, const byte_t row_counter) {
  return base_address + ((row_counter & 7) * 0x80) + ((row_counter >> 3) * 0x28);
}
