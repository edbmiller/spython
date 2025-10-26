#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef enum {
  PY_INT,
  PY_CODE,
  PY_FUNC,
  PY_CFUNC,
} PyType;

// NOTE: this is what goes in the hashtables :3
typedef struct PyObject {
  PyType type;
} PyObject;

typedef struct PyIntObject {
  PyType type;
  int value;
} PyIntObject;

typedef struct PyCodeObject {
  PyType type;
  char **bytecode; // array of bytecode instructions
  PyObject **consts;  // e.g. literals, compiled function/code objects
  char **argnames;
} PyCodeObject;

typedef struct PyFuncObject {
  PyType type;
  PyCodeObject *code;
} PyFuncObject;

typedef PyObject *(*PyCFunction)(PyObject *arg);

typedef struct PyCFuncObject {
  PyType type;
  PyCFunction function; // pointer to a C function
} PyCFuncObject;

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
