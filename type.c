#include <stdlib.h>

#include "type.h"
#include "cfunc.h"
#include "hash-table.h"

PyTypeObject py_type_type = {
  .base = { .type = &py_type_type },
  .method_defs = NULL,
  .methods = NULL,
};

// initialisation - run for all built-ins
void py_type_init(PyTypeObject *py_type_obj) {
  // create methods hash-table from method_defs 
  HashTable *methods = malloc(sizeof(HashTable)); 
  hashtable_init(methods); 
  
  // for each method definition...
  for (int i=0; py_type_obj->method_defs[i].name != NULL; i++) {
    // allocate the cfunc object
    PyCFuncObject *_method = malloc(sizeof(PyCFuncObject));
    _method->base.type = &py_type_cfunc;  
    _method->function = py_type_obj->method_defs[i].method;
    // and insert
    hashtable_insert(methods, py_type_obj->method_defs[i].name, (PyObject *) _method);
  }

  // and attach 
  py_type_obj->methods = methods;
  return;
}

