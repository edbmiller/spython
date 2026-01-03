#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash-table.h" 
#include "parser.h"

#define MAX_STACK_SIZE 100 
#define MAX_RECURSION_DEPTH 1000

// NOTE: value stack (each frame has one)
typedef struct {
  PyObject *data[MAX_STACK_SIZE];
  int top;
} Stack; 

void stack_init(Stack *s) {
  s->top = -1;
}

int stack_is_empty(Stack *s) {
  return s->top == -1;
}

int stack_is_full(Stack *s) {
  return s->top == MAX_STACK_SIZE - 1;
}

void stack_push(Stack *s, PyObject *object) {
  if (stack_is_full(s)) {
    printf("Stack overflow!\n");
    exit(1);
  } else
    s->data[++s->top] = object;
}

PyObject *stack_pop(Stack *s) {
  if (stack_is_empty(s)) {
    printf("Stack underflow!\n");
    exit(1);
  } else
    return s->data[s->top--];
}

/* TODO: fix after breaking with PyObjects
void stack_print(Stack *s) {
  if (stack_is_empty(s))
    printf("[]\n");
  else {
    int i;
    printf("[%d", *s->data[0]); 
    for (i = 1; i <= s->top; i++)
      printf(", %d", *s->data[i]);
    printf("]\n");
  }
}
*/

typedef struct PyFrameObject {
  struct PyCodeObject *code; // bytecode being executed
  int bytecode_offset; // program counter
  struct PyFrameObject *prev; // previous frame
  HashTable *locals;
  Stack *value_stack;
} PyFrameObject;

void py_frame_object_init(PyFrameObject *frame, HashTable *locals) {
  frame->bytecode_offset = 0;
  frame->prev = NULL;
  frame->value_stack = malloc(sizeof(Stack));
  stack_init(frame->value_stack);
  // in module-level frame we pass in our own locals (pointer to module dict)
  if (locals == NULL) {
    frame->locals = malloc(sizeof(HashTable));
    hashtable_init(frame->locals);
  } else {
    frame->locals = locals; 
  }
}

typedef struct PyState {
  PyFrameObject *current_frame;
  int recursion_depth;
  HashTable *globals;
} PyState;  

// NOTE: define opcodes
typedef enum {
  OP_UNKNOWN = -1,
  OP_LOAD_CONST,
  OP_STORE_NAME,
  OP_LOAD_NAME,
  OP_BINARY_OP,
  OP_MAKE_FUNCTION,
  OP_CALL_FUNCTION, // called with int argn, number of args to pop from stack
  OP_RETURN,
  OP_POP_TOP,
  OP_COMPARE,
  OP_JUMP, // arg: target offset
  OP_POP_JUMP_IF_FALSE // arg: target offset
} OpCode;


typedef PyBoolObject *(*CompareFunc)(PyIntObject *a, PyIntObject *b);

static PyBoolObject *compare_equals(PyIntObject *a, PyIntObject *b) {
  PyBoolObject *result = malloc(sizeof(PyBoolObject));
  if (a->value == b->value)
    result->value = 1;
  else
    result->value = 0;

  return result;
}

static PyBoolObject *compare_greater_than(PyIntObject *a, PyIntObject *b) {
  PyBoolObject *result = malloc(sizeof(PyBoolObject));
  if (a->value > b->value)
    result->value = 1;
  else
    result->value = 0;

  return result;
}

static PyBoolObject *compare_less_than(PyIntObject *a, PyIntObject *b) {
  PyBoolObject *result = malloc(sizeof(PyBoolObject));
  if (a->value > b->value)
    result->value = 1;
  else
    result->value = 0;

  return result;
}

static CompareFunc compare_func_table[3] = {
  compare_equals,
  compare_greater_than,
  compare_less_than
};

int equals_opcode(const char *input, size_t len, const char *cmd) {
  return (strlen(cmd) == len) && (strncmp(input, cmd, len) == 0);
}

OpCode get_opcode(const char *input) {
  int i = 0;
  while (input[i] != ',' && input[i] != '\0')
    i++;
  
  // TODO: use get_opcode and switch-case
  if (equals_opcode(input, i, "LOAD_CONST"))
    return OP_LOAD_CONST;
  else if (equals_opcode(input, i, "BINARY_OP"))
    return OP_BINARY_OP;
  else if (equals_opcode(input, i, "RETURN"))
    return OP_RETURN;
  else if (equals_opcode(input, i, "LOAD_NAME"))
    return OP_LOAD_NAME;
  else if (equals_opcode(input, i, "STORE_NAME"))
    return OP_STORE_NAME;
  else if (equals_opcode(input, i, "MAKE_FUNCTION"))
    return OP_MAKE_FUNCTION;
  else if (equals_opcode(input, i, "CALL_FUNCTION"))
    return OP_CALL_FUNCTION;
  else if (equals_opcode(input, i, "POP_TOP"))
    return OP_POP_TOP;
  else if (equals_opcode(input, i, "COMPARE"))
    return OP_COMPARE;
  else if (equals_opcode(input, i, "POP_JUMP_IF_FALSE"))
    return OP_POP_JUMP_IF_FALSE;
  else if (equals_opcode(input, i, "JUMP"))
    return OP_JUMP;
  else
    return OP_UNKNOWN;
}

int get_operand(const char *input) {

  int c;
  int i = 0;
  while ((c = input[i]) != ',') {
    // printf("DEBUG: found %c\n", c);
    if (c == '\0')
      return -1; // no operand
    i++;
  }
   
  // printf("DEBUG: finding operand...\n");
  char operand[5];
  int j = 0;
  i++;
  while ((c = input[i]) != '\0') {
    operand[j] = c;
    j++;
    i++;
  }

  // printf("DEBUG: operand = %s\n", operand);
  return atoi(operand);
}

char *get_string_operand(const char *input) {

  int c;
  int i = 0;
  while ((c = input[i]) != ',') {
    if (c == '\0')
      return NULL;
    i++;
  }

  // now strip name from '---' bit
  char *operand = malloc(10);
  int j = 0;
  i++; i++;
  while ((c = input[i]) != '\'') {
    operand[j] = c;
    j++;
    i++;
  }
  operand[j] = '\0';

  return operand;  
}

void handle_bytecode(PyState *state, const char *input) {
  // take a bytecode instruction and mutate frame and/or its stack
  OpCode opcode = get_opcode(input); 
  char *varname;
  // printf("DEBUG: handling bytecode %s\n", input);
  switch (opcode) { 
    case OP_LOAD_CONST: {
      // get obj from pre-compiled consts array
      int idx = get_operand(input); 
      PyObject *constant = state->current_frame->code->consts[idx];
      stack_push(state->current_frame->value_stack, constant);
      state->current_frame->bytecode_offset += 1; // move us forward one instruction
      break;
    }
    case OP_STORE_NAME: {
      // save stack[-1] to variable named operand
      PyObject *top = stack_pop(state->current_frame->value_stack);
      varname = get_string_operand(input);
      hashtable_insert(state->current_frame->locals, varname, top); // in bottom frame this points to globals
      state->current_frame->bytecode_offset += 1;
      break;
    }
    case OP_LOAD_NAME:
      // get variable name
      varname = get_string_operand(input); 
      if (varname == NULL) {
        printf("error: bad operand\n");
        exit(1);
      }
      // lookup in locals then globals
      PyObject *localobject = hashtable_get(state->current_frame->locals, varname);
      if (localobject != NULL) {
        stack_push(state->current_frame->value_stack, localobject);
      } else {
        PyObject *globalobject = hashtable_get(state->globals, varname); 
        if (globalobject != NULL) {
          stack_push(state->current_frame->value_stack, globalobject);
        } else {
          printf("NameError: name '%s' is not defined\n", varname);
          exit(1);
        }
      }
      state->current_frame->bytecode_offset += 1;
      break;
    case OP_BINARY_OP: {
      // get binary operator
      int bin_op = get_operand(input); 
      if (bin_op == ADD) {
        // ADD ------------
        PyObject *b = stack_pop(state->current_frame->value_stack);
        PyObject *a = stack_pop(state->current_frame->value_stack);
        if (a->type == PY_INT && b->type == PY_INT) {
          PyIntObject *result = malloc(sizeof(PyIntObject));
          result->value = ((PyIntObject *) a)->value + ((PyIntObject *) b)->value;
          stack_push(state->current_frame->value_stack, (PyObject *) result); 
          state->current_frame->bytecode_offset += 1;
          break;
        } else {
          printf("TypeError: can't add non-ints\n");    
          exit(1); 
        }
      } else if (bin_op == SUB) {
        // SUBTRACT ------
        PyObject *b = stack_pop(state->current_frame->value_stack);
        PyObject *a = stack_pop(state->current_frame->value_stack);
        if (a->type == PY_INT && b->type == PY_INT) {
          PyIntObject *result = malloc(sizeof(PyIntObject));
          result->value = ((PyIntObject *) a)->value - ((PyIntObject *) b)->value;
          stack_push(state->current_frame->value_stack, (PyObject *) result); 
          state->current_frame->bytecode_offset += 1;
          break;
        } else {
          printf("TypeError: can't subtract non-ints\n");    
          exit(1); 
        }
      } else if (bin_op == MULT) {
        // MULTIPLY ------
        PyObject *b = stack_pop(state->current_frame->value_stack);
        PyObject *a = stack_pop(state->current_frame->value_stack);
        if (a->type == PY_INT && b->type == PY_INT) {
          PyIntObject *result = malloc(sizeof(PyIntObject));
          result->value = ((PyIntObject *) a)->value * ((PyIntObject *) b)->value;
          stack_push(state->current_frame->value_stack, (PyObject *) result); 
          state->current_frame->bytecode_offset += 1;
          break;
        } else {
          printf("TypeError: can't multiply non-ints\n");    
          exit(1); 
        }
      } else if (bin_op == DIV) {
        // DIVIDE ------
        PyObject *b = stack_pop(state->current_frame->value_stack);
        PyObject *a = stack_pop(state->current_frame->value_stack);
        if (a->type == PY_INT && b->type == PY_INT) {
          PyIntObject *result = malloc(sizeof(PyIntObject));
          result->value = ((PyIntObject *) a)->value / ((PyIntObject *) b)->value;
          stack_push(state->current_frame->value_stack, (PyObject *) result); 
          state->current_frame->bytecode_offset += 1;
          break;
        } else {
          printf("TypeError: can't divide non-ints\n");    
          exit(1); 
        }
      } else if (bin_op == EQ) {
        // EQUALS ------
        PyObject *b = stack_pop(state->current_frame->value_stack);
        PyObject *a = stack_pop(state->current_frame->value_stack);
        if (a->type == PY_INT && b->type == PY_INT) {
          PyBoolObject *result = malloc(sizeof(PyBoolObject));
          result->value = ((PyIntObject *) a)->value == ((PyIntObject *) b)->value;
          printf("DEBUG: comparing %d to %d\n", ((PyIntObject *) a)->value, ((PyIntObject *) b)->value);
          stack_push(state->current_frame->value_stack, (PyObject *) result); 
          state->current_frame->bytecode_offset += 1;
          break;
        } else {
          printf("TypeError: can't compare non-ints\n");    
          exit(1); 
        }
      }
    }
    case OP_MAKE_FUNCTION: {
      // make func obj
      PyFuncObject *new_func = malloc(sizeof(PyFuncObject));
      new_func->type = PY_FUNC;
      new_func->code = (PyCodeObject *) stack_pop(state->current_frame->value_stack); 
      // push to stack - next opcode will be STORE_NAME...
      stack_push(state->current_frame->value_stack, (PyObject *) new_func);
      state->current_frame->bytecode_offset += 1;
      break;
    }
    case OP_CALL_FUNCTION:
      // push a new frame to callstack, remembering our
      // current bytecode offset in the current frame
      // NOTE: top of value stack needs to be a function lol
      if (state->recursion_depth == MAX_RECURSION_DEPTH) {
        printf("RecursionError: maximum recursion depth exceeded\n");
        exit(1);
      }

      int arg_count = get_operand(input);
      // pop #arg_count args
      PyObject **args = malloc(5 * sizeof(PyObject *));
      for (int i=arg_count-1; i>=0; i--) {
        args[i] = malloc(sizeof(PyObject *));
        args[i] = stack_pop(state->current_frame->value_stack);
      }
      // pop callable
      PyObject *f = stack_pop(state->current_frame->value_stack); 
      if (f->type == PY_FUNC) {
        // python functions
        PyFuncObject *func = (PyFuncObject *) f;
        // init new frame and populate locals
        PyFrameObject *new_frame = malloc(sizeof(PyFrameObject));
        py_frame_object_init(new_frame, NULL);
        for (int j=0; j<arg_count; j++) {
          hashtable_insert(new_frame->locals, func->code->argnames[j], args[j]);
        }
        new_frame->prev = state->current_frame;
        new_frame->code = func->code;
        // increment pc on the current frame so we pop back to the next instruction
        state->current_frame->bytecode_offset += 1;
        state->current_frame = new_frame;
        state->recursion_depth += 1;
      } else if (f->type == PY_CFUNC) {
        PyCFuncObject *cfunc = (PyCFuncObject *) f;
        // malloc args tuple
        PyTupleObject *py_args = malloc(sizeof(PyTupleObject));
        py_args->type = PY_TUPLE;
        py_args->elements = malloc(arg_count * sizeof(PyObject *));
        for (int i=0; args[i] != NULL; i++)
          py_args->elements[i] = args[i];
        PyObject *result = cfunc->function(py_args);
        stack_push(state->current_frame->value_stack, result);
        state->current_frame->bytecode_offset += 1; 
      }
      break;
    case OP_RETURN: {
      // pop a frame from the callstack, return to the
      // bytecode instruction referenced in the caller frame
      // (and push the return'd value to the value stack of
      // of the frame below)
      PyObject *return_value = stack_pop(state->current_frame->value_stack);
      PyFrameObject *old_frame = state->current_frame;
      // jump to prev frame and push the return value
      state->current_frame = old_frame->prev;
      stack_push(state->current_frame->value_stack, return_value);
      state->recursion_depth -= 1;
      // TODO: deallocate old frame!
      break;
    }
    case OP_POP_TOP:
      stack_pop(state->current_frame->value_stack);
      break;
    case OP_COMPARE: {
      // e.g. "COMPARE,0" means '=='
      // compare and push bool result
      PyIntObject *right = (PyIntObject *) stack_pop(state->current_frame->value_stack);
      PyIntObject *left = (PyIntObject *) stack_pop(state->current_frame->value_stack); 
      int comparison = get_operand(input);
      PyBoolObject *result = compare_func_table[comparison](left, right);
      stack_push(state->current_frame->value_stack, (PyObject *) result);
      state->current_frame->bytecode_offset += 1;
      break;
    }
    case OP_POP_JUMP_IF_FALSE: {
      PyBoolObject *top_bool = (PyBoolObject *) stack_pop(state->current_frame->value_stack);
      int target = get_operand(input); // jump target offset
      if (top_bool->value == 0)
        state->current_frame->bytecode_offset = target;
      else
        state->current_frame->bytecode_offset += 1;
      break;
    }
    case OP_JUMP:
      state->current_frame->bytecode_offset = get_operand(input);
      break;
    case OP_UNKNOWN:
      printf("error: bad opcode %s\n", input);
      exit(1); 
  }
}

char *read_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  if (!f) return NULL;

  // seek EOF
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);

  // alloc buffer
  char *buffer = malloc(size + 1);
  if (!buffer) {
    fclose(f);
    return NULL;
  }

  // read the file
  size_t read_size = fread(buffer, 1, size, f);
  buffer[read_size] = '\0';
  fclose(f);
  return buffer;
}

// BUILT-INS
PyObject *py_builtin_print(PyTupleObject *args) {
  for (int i=0; args->elements[i] != NULL; i++) { 
    switch (args->elements[i]->type) {
      case PY_INT:
        printf("%d", ((PyIntObject *) args->elements[i])->value);
        break;
      case PY_CODE:
        printf("error: print not defined for code objects\n");
        break;
      case PY_FUNC:
        printf("error: print not defined for functions\n");
        break;
      case PY_CFUNC:
        printf("error: print not defined for builtins\n");
        break;
      case PY_BOOL:
        printf("error: print not defined for bools\n");
        break;
      case PY_TUPLE:
        printf("error: print not defined for tuples\n");
        break;
    }
    if (args->elements[i+1] != NULL)
      printf(" ");
    else
      printf("\n");
  }
  return NULL;
}

int main(int argc, char **argv) {

  // initialise globals hash-table
  HashTable globals;
  hashtable_init(&globals);
  
  // load builtins
  PyCFunction py_builtin_print_function = py_builtin_print;
  PyCFuncObject *py_builtin_print_object = malloc(sizeof(PyCFuncObject));
  py_builtin_print_object->type = PY_CFUNC;
  py_builtin_print_object->function = py_builtin_print_function;
  hashtable_insert(&globals, "print", (PyObject *) py_builtin_print_object);

  // initialise state with current frame
  PyFrameObject bottom_frame;
  py_frame_object_init(&bottom_frame, &globals);

  PyState state; // essentially interpreter state
  state.current_frame = &bottom_frame; 
  state.recursion_depth = 0;
  state.globals = &globals;

  if (argv[1] == NULL) {
    printf("interactive mode unsupported! give me a file..\n");  
    exit(1);
  }

  // -> walk into a total string array of instructions
  char *input = read_file(argv[1]);
  Token *tokens = tokenize(input);
  // print_tokens(tokens);

  int t_idx = 0;
  Module *module = parse(tokens, &t_idx);
  printf("ast = \n");
  module_print(module);
  printf("\n");
  
  PyCodeObject *code = module_walk(module);
  state.current_frame->code = code;

  printf("bytecode =\n");
  for (int k=0; code->bytecode[k] != NULL; k++) {
    printf("%d: %s\n", k, code->bytecode[k]);
  }
  printf("\n");
  printf("output = \n");
  // -> interpret the bytecode - handle_bytecode increments program counter
  int i = state.current_frame->bytecode_offset;
  while (state.current_frame->code->bytecode[i] != NULL) {
    handle_bytecode(&state, state.current_frame->code->bytecode[i]);
    i = state.current_frame->bytecode_offset; // current_frame may have changed!
  }

  return 0;
}
