#include <stdio.h>
#include <stdlib.h>

#include "hash-table.h"
#include "type.h"
#include "cfunc.h"
#include "int.h"

int main() {
  // initialise built-ins
  py_type_init(&py_type_int);

  // create two ints and add them
  PyIntObject a = {
    .base = { .type = &py_type_int },
    .value = 4
  };

  PyIntObject b = {
    .base = { .type = &py_type_int },
    .value = 5
  };

  printf("doing add...\n");
  printf("method table is at %p\n", a.base.type->methods);
  PyObject *_int_add = hashtable_get(a.base.type->methods, "__add__");
  printf("looked up func at %p\n", _int_add); 
  PyObject *result = (((PyCFuncObject *) _int_add)->function)((PyObject *) &a, (PyObject * ) &b);
  printf("result = %d\n", ((PyIntObject *) result)->value);
}
