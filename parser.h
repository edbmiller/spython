#ifndef PARSER_H
#define PARSER_H

// NOTE: define our nodes
typedef enum NodeType {
  CONSTANT,
  NAME,
  BINARYADD,
  ASSIGN,
  RETURN
} NodeType;

typedef struct Constant {
  int value; // TODO: pointer
} Constant;

typedef struct Name {
  char *id;
} Name;

typedef struct Assign {
  Name *target;
  struct Node *value;
} Assign;

typedef struct BinaryAdd {
  struct Node *left;
  struct Node *right;
} BinaryAdd;

typedef struct Node {
  NodeType type;
  union {
    Constant *constant;
    Name *name;
    BinaryAdd *binary_add;
    Assign *assign;
  } data;
} Node;

typedef struct Module {
  Node *nodes[20];
} Module;

void module_print(Module *m);
void module_walk(Module *m, char **output, int *offset);
Module *parse(const char *input); // main entry point

#endif
