#include <stdlib.h>

#include "type.h"

PyTypeObject py_type_code = {
  .base = { .type = &py_type_type },
  .name = "code",
  .method_defs = NULL,
  .methods = NULL
};
