A very inefficient implementation of a small subset of Python, inspired by CPython

## Usage

file mode:
```
$ spython main.py # main.py: `x = 1 + 2\nprint(x)\n`
3
```

interactive (bytecode) mode:
```
$ spython
>> LOAD_CONST,3
Stack: [3]
Variables: {}

>> STORE_NAME,'x'
Stack: []
Variables: {'x':'3',}

>> LOAD_CONST,2
Stack: [2]
Variables: {'x':'3',}

>> LOAD_NAME,'x'
Stack: [2, 3]
Variables: {'x':'3',}

>> ADD
Stack: [5]
Variables: {'x':'3',}
```

## Current

The stack machine currently supports the following opcodes:
 - LOAD_CONST
 - STORE_NAME
 - LOAD_NAME
 - ADD
 - MAKE_FUNCTION

...and next we'll implement CALL_FUNCTION and RETURN with a callstack. We have a FunctionDef AST node with a name and instruction offset which emits MAKE_FUNCTION and STORE_NAME,\<name\> after its code logic bytecodes have been emitted. 

Basic idea: we remember the instruction offset we've jumped from in the caller frame - when we return, we push the return'd value onto the value stack of the frame "below", and jump to the remembered instruction.

## To-do

 - fix memory leaks
 - add functions
 - add some builtins e.g. `print`
 - add reference counting
