#ifndef TYPE_H
#define TYPE_H

#include "hash-table.h"

void py_type_init(PyTypeObject *py_type_obj);
extern PyTypeObject py_type_type;

#endif
