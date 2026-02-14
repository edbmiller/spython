#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "string.h"

void string_init(String *s) {
  const int INITIAL_SIZE = 8;
  s->length = 0;
  s->size = INITIAL_SIZE;
  s->data = malloc(INITIAL_SIZE);
  s->data[0] = '\0';
}

void string_append(String *s, const char *suffix) {
  size_t suffix_length = strlen(suffix);
  while (s->length + suffix_length > s->size) {
    // realloc
    size_t new_size = 2 * s->size;
    s->data = realloc(s->data, new_size);
    s->size = new_size;
  }
  // now copy and null-terminate
  memcpy(s->data + s->length, suffix, suffix_length);
  s->length += suffix_length;
  s->data[s->length] = '\0';
}

void string_appendf(String *s, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  va_list args_copy; 
  va_copy(args_copy, args);
  int needed = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  while (s->length + needed > s->size) {
    size_t new_size = 2 * s->size;
    s->data = realloc(s->data, new_size);
    s->size = new_size;
  }
  // now copy (this null-terminates by itself)
  int _ = vsnprintf(s->data + s->length, needed + 1, fmt, args_copy);
  va_end(args_copy);
  s->length += needed;
}

