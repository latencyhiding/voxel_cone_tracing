#pragma once
#include <stdint.h>

typedef struct
{
  uint8_t* ptr;
  size_t used;
  size_t size;
} buffer_t;
