#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "bytes.h"
#include "type.h"
#include "int.h"

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

PyObject *py_bytes_multiply(PyObject *a, PyObject *b) {
  assert(b->type == &py_type_int);
  int a_size = ((PyBytesObject *) a)->size;
  int b_value = ((PyIntObject *) b)->value;
  PyBytesObject *result = malloc(sizeof(PyBytesObject));
  result->base.type = &py_type_bytes;
  result->size = ((PyBytesObject *) a)->size * b_value;
  char *data = malloc(result->size + 1);
  if (data == NULL) {
    printf("MemoryError\n");
    exit(1);
  }
  result->data = data;
  // do memcpy (n - 1) times
  for (int i=0; i<b_value-1; i++) {
    memcpy(result->data + i*a_size, ((PyBytesObject *) a)->data, a_size); 
  }
  // and a strcpy to null-terminate
  if (b_value != 0) {
    strcpy(result->data + (b_value-1)*a_size, ((PyBytesObject *) a)->data);
  } else {
    result->data[0] = '\0'; 
  }
  return (PyObject *) result;
}

PyObject *py_bytes_length(PyObject *a, PyObject *b) {
  PyIntObject *result = malloc(sizeof(PyIntObject));
  result->base.type = &py_type_int;
  result->value = ((PyBytesObject *) b)->size;
  return (PyObject *) result;
}

PyMethodDef bytes_method_defs[] = {
  { "__add__", py_bytes_add },
  { "__mult__", py_bytes_multiply },
  { "__len__", py_bytes_length },
  { NULL, NULL }
};

PyTypeObject py_type_bytes = {
  .base = { .type = &py_type_type },
  .name = "str",
  .method_defs = bytes_method_defs,
  .methods = NULL
};
