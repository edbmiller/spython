
#ifndef STRING_H
#define STRING_H

typedef struct {
  char *data;
  int length;
  int size;
} String;

void string_init(String *s);
void string_append(String *s, const char *suffix);
void string_appendf(String *s, const char *fmt, ...);

#endif
