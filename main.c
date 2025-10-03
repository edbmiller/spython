#include <stdio.h>
#include <stdlib.h>

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

int main() {
  Stack s;
  stack_init(&s);
  printf("Stack initialised! top = %d\n", s.top);
  
  // push and pop!
  stack_push(&s, 3);
  stack_push(&s, 2);
  printf("Popped: %d\n", stack_pop(&s));
  stack_push(&s, 5);
  printf("End stack: ");
  stack_print(&s);
  printf("\n");
  
  return 0;
}
