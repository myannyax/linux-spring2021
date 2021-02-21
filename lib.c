#include <stdio.h>
#include <stdlib.h>
#include "lib.h"

static size_t N = 64 * 1024;

static char memory[64 * 1024];
static size_t c_offset = 0;

void *__malloc(size_t size) {
  fprintf(stderr, "%zu\n", size);
  if (c_offset + size + 1 >= N) {
    return NULL;
  }
  memory[c_offset] = size;
  c_offset += 1;

  char *pointer = memory + c_offset;
  c_offset += size;
  return pointer;
}


void *malloc(size_t size) {
  fprintf(stderr, "malloc\n");
  return __malloc(size);
}

void *calloc(size_t num, size_t size) {
  fprintf(stderr, "calloc\n");
  return __malloc(size * num);
}

void *realloc(void *ptr, size_t size) {
  fprintf(stderr, "realloc\n");
  if (size == 0) {
    fprintf(stderr, "0\n");
    return NULL;
  }

  if (!ptr) {
    return __malloc(size);
  }

  void* pointer = __malloc(size);
  if (!pointer) {
    return NULL;
  }

  memcpy(pointer, ptr, *((char*)ptr--));

  return pointer;
}

void free(void *ptr) {
  fprintf(stderr, "free\n");
}

