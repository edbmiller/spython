#include <stdlib.h>

#include "tuple.h"
#include "type.h"

PyTypeObject py_type_tuple = {
  .base = { .type = &py_type_tuple },
  .name = "tuple",
  .method_defs = NULL,
  .methods = NULL,
};
