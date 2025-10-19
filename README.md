A very inefficient implementation of a small subset of Python, inspired by CPython

## Usage

file mode:
```
$ spython main.py # main.py: `x = 1 + 2\nprint(x)\n`
3
```

## Current

The stack machine currently supports the following opcodes:
 - LOAD_CONST
 - STORE_NAME
 - LOAD_NAME
 - ADD
 - MAKE_FUNCTION
 - RETURN
 - CALL_FUNCTION

We have a FunctionDef AST node with a name and body, which emits (in this order):
 - MAKE_FUNCTION,\<int\> with arg the int offset of the line after the final line of the function body
 - bytecode instructions for the function body
 - STORE_NAME,\<str\> with arg the function name

On MAKE_FUNCTION opcodes, the stack VM will:
 - allocate a PyFuncObject with a reference `function_start_offset` to the int offset of the first instruction of the function body
 - push this object onto stack

...and after the subsequent STORE_NAME, we insert into the variables hashtable (after casting to PyObject).

## To-do

 - implement stack machine logic for CALL_FUNCTION

## To-do eventually...

 - fix memory leaks
 - add some builtins e.g. `print`
 - add reference counting
