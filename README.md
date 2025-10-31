A very inefficient implementation of a small subset of Python, inspired by CPython

## Working on...

3a. Done! Support if blocks with int comparisons and POP_JUMP_IF_FALSE
b. Support else
c. Support elif

How does if work? We compile if-else blocks to the following:

```
<comparison expression bytecode>
POP_JUMP_IF_FALSE <int>
<true branch bytecode>
JUMP <int> // jump past false branch
<false branch bytecode>
...
<rest of instructions>
```

where one of the blocks may not exist if we have no else. Any if/elseif/else sequence just gets compiled to a sequence of true/false blocks and some POP_JUMP_IF_FALSE and JUMP commands to link them together!

## Details (...needs updating)

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

 - free our memory at some point :P
 - refcounting
