#include <stdlib.h>

#include "func.h"
#include "type.h"

PyTypeObject py_type_func = {
  .base = { .type = &py_type_type },
  .name = "function",
  .method_defs = NULL,
  .methods = NULL,
};
