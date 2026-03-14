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

typedef enum { TEXT, GRAPHICS } video_mode_e;
typedef enum { HI, LO } res_e;

typedef enum { PRIM, SEC } page_e;

static byte_t keyboard_latch = 0;
static bool keyboard_ready = false;

static video_mode_e video_mode = TEXT;
static page_e page = SEC;
static res_e res = LO;

bool apple2_terminal_should_render = true;

byte_t apple2_memory_read(const memory_t *self, const word_t address, bool *success) {
  if (IS_INVALID_MEMORY(self) || address >= self->memory_size) {
    *success = false;
    return 0;
  }

  *success = true;

  /* ===================== KEYBOARD ===================== */

  switch (address) {
    case 0xC050:
      emu_log(INFO, "Set graphics; ");
      video_mode = GRAPHICS;
      return 0;
    case 0xC051:
      emu_log(INFO, "Set text; ");
      video_mode = TEXT;
      return 0;
    case 0xC054:
      emu_log(INFO, "Set primary page; ");
      page = PRIM;
      return 0;
    case 0xC055:
      emu_log(INFO, "Set secondary page; ");
      page = SEC;
      return 0;
    case 0xC056:
      emu_log(INFO, "Set lo mode; ");
      res = LO;
      return 0;
    case 0xC057:
      emu_log(INFO, "Set hi mode; ");
      res = HI;
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
  /* ===================== ROM ===================== */

  if (address >= ROM_START) {
    return self->memory[address];
  }

  /* ===================== RAM ===================== */

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

  /* ===================== ROM PROTECTION ===================== */

  if (address >= ROM_START) {
    emu_log(WARN, "Write to ROM %04X ignored\n", address);
    return;
  }

  /* ===================== RAM ===================== */

  if (address < IO_START) {
    self->memory[address] = value;
    return;
  }

  emu_log(WARN, "Unhandled I/O write %04X value %02X\n", address, value);
}

// void render(memory_t *memory, const page_e page) {
//   bool suc;
//
//   printf("\x1B[2J\x1B[1;1H");
//
//   const log_level_e saved = g_log_level;
//   g_log_level = NO_LOG;
//
//   puts("\nAPPLE II terminal\n");
//
//   for (int i = 0; i < 40; ++i) {
//     putchar('=');
//   }
//
//   putchar(0xa);
//
//   int count = 0;
//
//   word_t start = 0, end = 0;
//
//   switch (page) {
//     case PRIM:
//       start = TEXT_START_PRIM;
//       end = TEXT_END_PRIM;
//       break;
//     case SEC:
//       start = TEXT_START_SEC;
//       end = TEXT_END_SEC;
//   }
//
//   for (int i = start; i <= end; ++i) {
//     byte_t symbol = memory_read(memory, i, &suc);
//
//     if (symbol == 0xEA) {
//       symbol = '.';
//     } else {
//       symbol &= 0x7F;
//
//       if (symbol < 0x20) {
//         symbol += 0x40;
//       }
//     }
//
//     if (!suc || !isprint(symbol)) {
//       symbol = ' ';
//     }
//
//     putchar(symbol);
//     ++count;
//
//     if (count == 1024) {
//       while (count % 40 != 0) {
//         putchar(' ');  // fill empty part of the last line with spaces
//         ++count;
//       }
//     }
//
//     if (count % 40 == 0) {
//       puts("|");
//     }
//   }
//
//   for (int i = 0; i < 40; ++i) {
//     putchar('=');
//   }
//
//   putchar(0xa);
//
//   g_log_level = saved;
// }
//

// void render(memory_t *memory, const page_e page) {
//   bool suc;
//
//   printf("\x1B[2J\x1B[1;1H");
//
//   const log_level_e saved = g_log_level;
//   g_log_level = NO_LOG;
//
//   puts("\nAPPLE II terminal\n");
//
//   for (int i = 0; i < 40; ++i) putchar('=');
//
//   putchar('\n');
//
//   word_t base;
//
//   switch (page) {
//     case PRIM:
//       base = TEXT_START_PRIM;
//       break;
//     case SEC:
//       base = TEXT_START_SEC;
//       break;
//   }
//
//   for (int row = 0; row < 24; row++) {
//     /* реальная адресация строк Apple II */
//     word_t row_addr = base + ((row & 0x07) << 7) +  // группы по 0x80
//                       ((row & 0x18) << 2);          // смещение группы
//
//     for (int col = 0; col < 40; col++) {
//       byte_t symbol = memory_read(memory, row_addr + col, &suc);
//
//       if (symbol == 0xEA) {
//         symbol = '.';
//       } else {
//         symbol &= 0x7F;
//
//         if (symbol < 0x20)
//           symbol += 0x40;
//       }
//
//       if (!suc || !isprint(symbol))
//         symbol = ' ';
//
//       putchar(symbol);
//     }
//
//     puts("|");
//   }
//
//   for (int i = 0; i < 40; ++i) putchar('=');
//
//   putchar('\n');
//
//   g_log_level = saved;
// }
void render(memory_t *memory, const page_e page) {
  bool suc;

  printf("\x1B[2J\x1B[1;1H");

  const log_level_e saved = g_log_level;
  g_log_level = NO_LOG;

  puts("\nAPPLE II terminal\n");

  for (int i = 0; i < 40; ++i) putchar('=');

  putchar('\n');

  word_t base = (page == PRIM) ? TEXT_START_PRIM : TEXT_START_SEC;

  for (int row = 0; row < 24; row++) {
    word_t row_addr = base + ((row & 7) * 0x80) +  // шаг внутри группы
                      ((row >> 3) * 0x28);         // смещение группы

    for (int col = 0; col < 40; col++) {
      byte_t symbol = memory_read(memory, row_addr + col, &suc);

      if (symbol == 0xEA) {
        symbol = '?';
      } else {
        symbol &= 0x7F;

        if (symbol < 0x20)
          symbol += 0x40;

        if (!suc || !isprint(symbol))
          symbol = ' ';
      }
      putchar(symbol);
    }

    puts("|");
  }

  for (int i = 0; i < 40; ++i) putchar('=');

  putchar('\n');

  g_log_level = saved;
}

void apple2_render(const memory_t *memory) {
  if (!apple2_terminal_should_render) {
    return;
  }

  render(memory, page);

  apple2_terminal_should_render = false;
}
