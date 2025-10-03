#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 100

// NOTE: value stack
// NOTE: just an int array and we track top index
typedef struct {
  int data[MAX_STACK_SIZE];
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

void stack_push(Stack *s, int value) {
  if (stack_is_full(s)) {
    printf("Stack overflow!\n");
    exit(1);
  } else
    s->data[++s->top] = value;
}

int stack_pop(Stack *s) {
  if (stack_is_empty(s)) {
    printf("Stack underflow!\n");
    exit(1);
  } else
    return s->data[s->top--];
}

void stack_print(Stack *s) {
  if (stack_is_empty(s))
    printf("[]");
  else {
    int i;
    printf("[%d", s->data[0]); 
    for (i = 1; i <= s->top; i++)
      printf(", %d", s->data[i]);
    printf("]");
  }
}

// TODO: implement LOAD_NAME with hash-set
// NOTE: define opcodes
typedef enum {
  OP_UNKNOWN = -1,
  OP_LOAD_CONST,
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
  // printf("DEBUG: got input: %s, %d\n", input, i);
  if (equals_opcode(input, i, "LOAD_CONST"))
    return OP_LOAD_CONST;
  else if (equals_opcode(input, i, "ADD"))
    return OP_ADD;
  else if (equals_opcode(input, i, "RETURN"))
    return OP_RETURN;
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

int main() {

  // initialise value stack
  Stack s;
  stack_init(&s);
  
  while (1) {

    // get bytecode commands from user
    printf(">> ");
    char input[20];
    scanf("%19s", input);

    // parse instruction
    OpCode opcode;
    int operand;

    opcode = get_opcode(input); 
    operand = get_operand(input); 

    // printf("Got opcode and operand! %d, %d\n", opcode, operand);
    switch (opcode) { 
      case OP_RETURN:
        printf("return: %d\n", stack_pop(&s));
        exit(0);
      case OP_LOAD_CONST:    
        // printf("pushing const: %d\n", operand);
        stack_push(&s, operand);
        break;
      case OP_ADD:
        int b = stack_pop(&s);
        int a = stack_pop(&s);
        stack_push(&s, a + b); 
        break;
      case OP_UNKNOWN:
        printf("error: bad opcode\n");
        exit(1); 
    }
    
    // TODO: take CLI argument to display stack or not
    printf("Stack: ");
    stack_print(&s);
    printf("\n\n");
  }

  return 0;
}
