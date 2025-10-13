#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef enum {
  PY_INT,
  PY_FUNC
} PyType;

// NOTE: this is what goes in the hashtables :3
typedef struct PyObject {
  PyType type;
} PyObject;

typedef struct PyIntObject {
  PyType type;
  int value;
} PyIntObject;

typedef struct PyFuncObject {
  PyType type;
  int bytecode_offset;
} PyFuncObject;

typedef struct Entry {
  char *key; // NOTE: points to first char
  PyObject *object; 
  struct Entry *next;
} Entry;

typedef struct {
  Entry *data[8]; // fixed size - TODO: resize depending on load factor
  int size;
  int itemCount;
} HashTable;

void hashtable_init(HashTable *htable);
void hashtable_insert(HashTable *htable, const char *key, PyObject *object);
PyObject *hashtable_get(HashTable *htable, const char *key);
void hashtable_print(HashTable *htable);

#endif
