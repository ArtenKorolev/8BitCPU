#include "file_io.h"

#include <stdio.h>

size_t get_file_size(FILE *stream);

file_content_t read_bin_file(const char *file_name) {
  file_content_t file_content;

  file_content.data = NULL;
  file_content.size = 0;

  FILE *stream = fopen(file_name, "rb");

  if (stream == NULL) {
    return file_content;
  }

  const size_t file_size = get_file_size(stream);

  if (file_size == 0) {
    fclose(stream);
    return file_content;
  }

  file_content.size = file_size;
  file_content.data = malloc(file_size);

  const size_t bytes_read = fread(file_content.data, 1, file_size, stream);

  if (bytes_read != file_size) {
    file_content.data = NULL;
    file_content.size = 0;
    fclose(stream);
    free(file_content.data);
    return file_content;
  }

  if (file_content.data == NULL) {
    fclose(stream);
    file_content.size = 0;
    return file_content;
  }

  fclose(stream);

  return file_content;
}

size_t get_file_size(FILE *stream) {
  if (fseek(stream, 0, SEEK_END) == 0) {
    size_t size = ftell(stream);
    fseek(stream, 0, SEEK_SET);
    return size;
  }

  return 0;
}

void file_content_free(file_content_t *self) {
  if (self->data == NULL || self->size == 0) {
    return;
  }

  free(self->data);
}
