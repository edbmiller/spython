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
  int bytecode_offset;
  struct PyFrameObject *f_back; // previous frame
  Stack *value_stack;
} PyFrameObject;

void py_frame_object_init(PyFrameObject *frame, int bytecode_offset) {
  frame->bytecode_offset = bytecode_offset;
  frame->f_back = NULL;
  frame->value_stack = malloc(sizeof(Stack));
  stack_init(frame->value_stack);
}

typedef struct PyState {
  PyFrameObject *current_frame;
  int recursion_depth;
} PyState;  

// NOTE: define opcodes
typedef enum {
  OP_UNKNOWN = -1,
  OP_LOAD_CONST,
  OP_STORE_NAME,
  OP_LOAD_NAME,
  OP_ADD,
  OP_MAKE_FUNCTION,
  OP_CALL_FUNCTION, // called with int argn, number of args to pop from stack
  OP_RETURN
} OpCode;

int equals_opcode(const char *input, size_t len, const char *cmd) {
  return (strlen(cmd) == len) && (strncmp(input, cmd, len) == 0);
}

OpCode get_opcode(const char *input) {
  int i = 0;
  while (input[i] != ',' && input[i] != '\0')
    i++;
  
  // TODO: use a hash-table
  if (equals_opcode(input, i, "LOAD_CONST"))
    return OP_LOAD_CONST;
  else if (equals_opcode(input, i, "ADD"))
    return OP_ADD;
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

void handle_bytecode(PyState *state, HashTable *vars, const char *input) {
  // take a bytecode instruction and mutate frame and/or its stack
  OpCode opcode = get_opcode(input); 
  char *varname;
  switch (opcode) { 
    case OP_LOAD_CONST:
      // create object
      PyIntObject *constant = malloc(sizeof(PyIntObject));
      constant->value = get_operand(input);
      stack_push(state->current_frame->value_stack, (PyObject *) constant);
      state->current_frame->bytecode_offset += 1; // move us forward one instruction
      break;
    case OP_STORE_NAME:
      // save stack[-1] to variable named operand
      PyObject *top = stack_pop(state->current_frame->value_stack);
      varname = get_string_operand(input);
      hashtable_insert(vars, varname, top);
      state->current_frame->bytecode_offset += 1;
      break;
    case OP_LOAD_NAME:
      // get variable name
      varname = get_string_operand(input); 
      if (varname == NULL) {
        printf("error: bad operand\n");
        exit(1);
      }
      // lookup in vars table
      PyObject *object = hashtable_get(vars, varname);
      if (object == NULL) {
        printf("NameError: name '%s' is not defined\n", varname);
        exit(1);
      }
      stack_push(state->current_frame->value_stack, object);
      state->current_frame->bytecode_offset += 1;
      break;
    case OP_ADD:
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
    case OP_MAKE_FUNCTION:
      int jump_offset = get_operand(input);
      // ^ first line after function body finishes
      PyFuncObject *new_func = malloc(sizeof(PyFuncObject));
      new_func->type = PY_FUNC;
      new_func->bytecode_offset = state->current_frame->bytecode_offset + 1;
      // push onto stack
      stack_push(state->current_frame->value_stack, (PyObject *) new_func);
      state->current_frame->bytecode_offset = jump_offset;
      break;
    case OP_CALL_FUNCTION:
      // push a new frame to callstack, remembering our
      // current bytecode offset in the current frame
      // NOTE: top of value stack needs to be a function lol
      if (state->recursion_depth == MAX_RECURSION_DEPTH) {
        printf("RecursionError: maximum recursion depth exceeded\n");
        exit(1);
      }
      PyFuncObject *func = (PyFuncObject *) stack_pop(state->current_frame->value_stack); 
      PyFrameObject *new_frame = malloc(sizeof(PyFrameObject));
      py_frame_object_init(new_frame, func->bytecode_offset);
      new_frame->f_back = state->current_frame;
      // increment pc on the current frame so we pop back to the next instruction
      state->current_frame->bytecode_offset += 1;
      state->current_frame = new_frame;
      state->recursion_depth += 1;
      break;
    case OP_RETURN:
      // pop a frame from the callstack, return to the
      // bytecode instruction referenced in the caller frame
      // (and push the return'd value to the value stack of
      // of the frame below)
      PyObject *return_value = stack_pop(state->current_frame->value_stack);
      PyFrameObject *old_frame = state->current_frame;
      // jump to "below" frame and push the return value
      state->current_frame = old_frame->f_back;
      stack_push(state->current_frame->value_stack, return_value);
      state->recursion_depth -= 1;
      // TODO: deallocate old frame!
      break;
    case OP_UNKNOWN:
      printf("error: bad opcode\n");
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

int main(int argc, char **argv) {

  // initialise variables hash-table
  HashTable vars;
  hashtable_init(&vars);

  // initialise state with current frame
  PyFrameObject bottom_frame;
  py_frame_object_init(&bottom_frame, 0);
  PyState state; // essentially interpreter state
  state.current_frame = &bottom_frame; 
  state.recursion_depth = 0;

  if (argv[1] == NULL) {
    printf("interactive mode unsupported! give me a file..\n");  
    exit(1);
  }

  // allocate string array of compiled bytecode instructions
  char **instructions = malloc(100 * sizeof(char *));
  int *offset = malloc(sizeof(int));

  // -> walk into a total string array of instructions
  char *input = read_file(argv[1]);
  int *distance = malloc(sizeof(int));
  Module *module = parse(input, 0, distance);
  module_walk(module, instructions, offset);

  // -> interpret the bytecode - handle_bytecode increments program counter
  int i = state.current_frame->bytecode_offset;
  while (instructions[i] != NULL) {
    printf("%d: %s\n", i, instructions[i]);
    handle_bytecode(&state, &vars, instructions[i]);
    i = state.current_frame->bytecode_offset; // current_frame may have changed!
  }

  return 0;
}
