A very inefficient implementation of a small subset of Python, inspired by CPython

## Working on...

1. Done! Support pure expressions (e.g a line like `print(3)` or `4 + 5`) 
2. Add PyTupleObject and pass args as a tuple to `print`
3. Support if/else with int comparisons and POP_JUMP_IF_FALSE

## Current

Now supports functions, e.g.

```py
# main.py
def foo(a, b):
    x = a + b
    return x

result = foo()
```

And we enforce a recursion limit:

```
# main.py
def foo():
    return foo()

$ spython main.py
RecursionError: maximum recursion depth exceeded
```

## Details

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

## To-do eventually...

 - fix memory leaks
 - add some builtins e.g. `print`
 - add reference counting
