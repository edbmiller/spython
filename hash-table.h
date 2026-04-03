#ifndef HASH_TABLE_H
#define HASH_TABLE_H

// forward declare so we can define PyObject
struct PyTypeObject;
struct HashTable;
struct PyMethodDef;

typedef struct PyObject {
  struct PyTypeObject *type;
} PyObject;

typedef struct PyTypeObject {
  PyObject base;
  struct PyMethodDef *method_defs;
  struct HashTable *methods; // constructed at startup from method_defs
} PyTypeObject;

typedef struct PyIntObject {
  PyObject base;
  int value;
} PyIntObject;

typedef struct PyBoolObject {
  PyObject base;
  int value; // 1 for True
} PyBoolObject;

typedef struct PyTupleObject {
  PyObject base;
  int size;
  PyObject **elements; // TODO: allocate inline with the [] trick
} PyTupleObject;

typedef struct PyBytesObject {
  PyObject base;
  int size;
  char *data; // TODO: make dynamic string, not char buffer...
} PyBytesObject;

typedef struct PyCodeObject {
  PyObject base;
  char **bytecode; // array of bytecode instructions
  PyObject **consts;  // e.g. literals, compiled function/code objects
  char **argnames; // TODO: allocate inline
} PyCodeObject;

typedef struct PyFuncObject {
  PyObject base;
  PyCodeObject *code;
} PyFuncObject;

// args should be a tuple
typedef PyObject *(*PyCFunction)(PyObject *self, PyObject *args);

typedef struct PyMethodDef {
  char *name;
  PyCFunction method; 
} PyMethodDef;

typedef struct PyCFuncObject {
  PyObject base;
  PyCFunction function; // pointer to a C function
} PyCFuncObject;

typedef struct Entry {
  char *key; // NOTE: points to first char
  PyObject *object; 
  struct Entry *next;
} Entry;

typedef struct HashTable {
  Entry *data[8]; // fixed size - TODO: resize depending on load factor
  int size;
  int itemCount;
} HashTable;

void hashtable_init(HashTable *htable);
void hashtable_insert(HashTable *htable, const char *key, PyObject *object);
PyObject *hashtable_get(HashTable *htable, const char *key);
void hashtable_print(HashTable *htable);

#endif
