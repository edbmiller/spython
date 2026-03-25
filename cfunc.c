#include <stdlib.h>

#include "cfunc.h"
#include "type.h"

PyTypeObject py_type_cfunc = {
  .base = { .type = &py_type_type },
  .method_defs = NULL,
  .methods = NULL,
};
