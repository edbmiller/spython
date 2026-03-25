#include <stdlib.h>

#include "int.h"
#include "type.h"
#include "hash-table.h"

PyObject *py_int_add(PyObject *a, PyObject *b) {
  PyIntObject *result = malloc(sizeof(PyIntObject));
  result->base.type = &py_type_int;
  result->value = ((PyIntObject *) a)->value + ((PyIntObject *) b)->value;
  return (PyObject *) result;
}

PyMethodDef int_method_defs[] = {
    { "__add__", py_int_add },
    { NULL, NULL }
};

PyTypeObject py_type_int = {
  .base = { .type = &py_type_type },
  .method_defs = int_method_defs,
  .methods = NULL
};

