#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct Entry {
  char *key; // NOTE: points to first char
  int *value; // NOTE: eventually will be PyObject *object
  struct Entry *next;
} Entry;

typedef struct {
  Entry *data[8]; // fixed size - TODO: resize depending on load factor
  int size;
  int itemCount;
} HashTable;

void hashtable_init(HashTable *htable);
void hashtable_insert(HashTable *htable, const char *key, int *value);
int *hashtable_get(HashTable *htable, const char *key);
void hashtable_print(HashTable *htable);

#endif
