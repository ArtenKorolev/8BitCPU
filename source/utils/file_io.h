#pragma once

#include <stdlib.h>

typedef struct {
  size_t size;
  char *data;
} file_content_t;

file_content_t read_bin_file(const char *file_name);
void file_content_free(file_content_t *self);
