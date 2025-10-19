A very inefficient implementation of a small subset of Python, inspired by CPython

## Usage

Now supports (argument-less :S) functions, e.g.

```py
# main.py
def foo():
    x = 1 + 2
    return x

a = foo()
```

executes the following bytecode sequence on the stack VM (original line numbers on the left):

```
0: MAKE_FUNCTION,7
7: STORE_NAME,'foo'
8: LOAD_NAME,'foo'
9: CALL_FUNCTION
1: LOAD_CONST,1
2: LOAD_CONST,2
3: ADD
4: STORE_NAME,'x'
5: LOAD_NAME,'x'
6: RETURN
10: STORE_NAME,'a'
```

And we enforce a recursion limit:

```
# main.py
def foo():
    return foo()

$ spython main.py
RecursionError: maximum recursion depth exceeded
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

## To-do eventually...

 - fix memory leaks
 - add some builtins e.g. `print`
 - add reference counting
