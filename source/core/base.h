#pragma once

#include <stdint.h>

typedef uint8_t byte_t;
typedef uint16_t word_t;

#define BYTE_MAX UINT8_MAX
#define WORD_MAX UINT16_MAX

#define MAKE_WORD(a, b) (((word_t)a << 8) | (word_t)(b))

#define KB_64 65536
#define MEMORY_SIZE KB_64
