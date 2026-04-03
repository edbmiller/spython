#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "type.h"

PyObject *py_bytes_add(PyObject *a, PyObject *b) {
  PyBytesObject *result = malloc(sizeof(PyBytesObject));
  result->base.type = &py_type_bytes;
  int a_size = ((PyBytesObject *) a)->size;
  int b_size = ((PyBytesObject *) b)->size;
  result->size = a_size + b_size;
  result->data = malloc(result->size + 1);
  memcpy(result->data, ((PyBytesObject *) a)->data, a_size);
  strcpy(result->data + a_size, ((PyBytesObject *) b)->data);
  return (PyObject *) result;
}

PyMethodDef bytes_method_defs[] = {
  { "__add__", py_bytes_add },
  { NULL, NULL }
};

PyTypeObject py_type_bytes = {
  .base = { .type = &py_type_type },
  .method_defs = bytes_method_defs,
  .methods = NULL
};
