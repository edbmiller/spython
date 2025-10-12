#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash-table.h"
#include "parser.h"

#define MAX_STACK_SIZE 100

// NOTE: value stack
// NOTE: just an int array and we track top index
typedef struct {
  int *data[MAX_STACK_SIZE]; // array of pointers now!
  int top;
} Stack; 

// NOTE: value stack methods
void stack_init(Stack *s) {
  s->top = -1;
}

int stack_is_empty(Stack *s) {
  return s->top == -1;
}

int stack_is_full(Stack *s) {
  return s->top == MAX_STACK_SIZE - 1;
}

void stack_push(Stack *s, int *value) {
  if (stack_is_full(s)) {
    printf("Stack overflow!\n");
    exit(1);
  } else
    s->data[++s->top] = value;
}

int *stack_pop(Stack *s) {
  if (stack_is_empty(s)) {
    printf("Stack underflow!\n");
    exit(1);
  } else
    return s->data[s->top--];
}

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

// NOTE: define opcodes
typedef enum {
  OP_UNKNOWN = -1,
  OP_LOAD_CONST,
  OP_STORE_NAME,
  OP_LOAD_NAME,
  OP_ADD,
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

void handle_bytecode(Stack *s, HashTable *vars, const char *input) {
  // take a bytecode instruction and mutate stack
  OpCode opcode = get_opcode(input); 
  char *varname;
  switch (opcode) { 
    case OP_RETURN:
      printf("return: %d\n", *stack_pop(s));
      exit(0);
    case OP_LOAD_CONST:    
      // malloc the object
      int *object = malloc(sizeof(int));
      *object = get_operand(input);
      stack_push(s, object);
      break;
    case OP_STORE_NAME:
      // save stack[-1] to variable named operand
      int *top = stack_pop(s);
      varname = get_string_operand(input);
      hashtable_insert(vars, varname, top);
      break;
    case OP_LOAD_NAME:
      // get variable name
      varname = get_string_operand(input); 
      if (varname == NULL) {
        printf("error: bad operand\n");
        exit(1);
      }
      // lookup in vars table
      int *value = hashtable_get(vars, varname);
      if (value == NULL) {
        printf("NameError: name '%s' is not defined\n", varname);
        exit(1);
      }
      stack_push(s, value);
      break;
    case OP_ADD:
      int *b = stack_pop(s);
      int *a = stack_pop(s);
      int *result = malloc(sizeof(int));
      *result = *a + *b;
      stack_push(s, result); 
      break;
    case OP_UNKNOWN:
      printf("error: bad opcode\n");
      exit(1); 
  }

  printf("Stack: ");
  stack_print(s);
  printf("Variables: ");
  hashtable_print(vars);
  printf("\n");
}

int main(int argc, char **argv) {

  // initialise value stack
  Stack s;
  stack_init(&s);
  
  // initialise variables hash-table
  HashTable vars;
  hashtable_init(&vars); 

  if (argv[1] == NULL) {
    while (1) {
      // get bytecode commands from user and handle
      printf(">> ");
      char input[20];
      scanf("%19s", input);
      handle_bytecode(&s, &vars, input);
    }
  } else {
    // allocate string array of compiled bytecode instructions
    char **instructions = malloc(100 * sizeof(char *));
    int *offset = malloc(sizeof(int));
    *offset = 0;
    int i = 0;
    // compile each line to a bytecode module and emit and handle instructions
    // NOTE: assume each line is a single module for now (e.g. no functions)
    FILE *file = fopen(argv[1], "r");
    char input[60]; // a line of input
    while (fgets(input, 60, file)) {
      Module *module = malloc(sizeof(Module));
      module = parse(input);
      module_walk(module, instructions, offset);
      // now handle each instruction
      for (; i<*offset; i++) {
        printf("%d: %s\n", i, instructions[i]);
        handle_bytecode(&s, &vars, instructions[i]);
      }
    }
  }

  return 0;
}
