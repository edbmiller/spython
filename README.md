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
 - RETURN

...but the AST parser only supports adding two constants, e.g. `x = 13 + 24`

## To-do

 - fix memory leaks
 - add functions
 - add some builtins e.g. `print`
 - add reference counting
