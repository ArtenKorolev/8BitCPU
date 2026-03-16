#pragma once

#include <stdbool.h>

#include "base.h"
#include "memory.h"

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

typedef struct {
  video_mode_e video_mode;
  page_e page;
  res_e res;
  bool term_render_request;
  byte_t keyboard_latch;
  bool keyboard_ready;
} apple2_data_t;

byte_t apple2_memory_read(const memory_t *self, word_t address, bool *success, void *data);
void apple2_memory_write(memory_t *self, word_t address, byte_t value, void *data);
void apple2_render(const memory_t *memory, void *data);
