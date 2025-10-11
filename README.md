A very inefficient implementation of a small subset of Python, inspired by CPython

## Current

The stack machine currently supports the following opcodes:
 - LOAD_CONST
 - STORE_NAME
 - LOAD_NAME
 - ADD
 - RETURN

...but the AST parser only supports adding two constants, e.g. `x = 13 + 24`

## To-do

 - fix memory leaks
 - add functions
 - add some builtins e.g. `print`
 - add reference counting
