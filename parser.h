#ifndef PARSER_H
#define PARSER_H

// NOTE: define our nodes
typedef enum NodeType {
  CONSTANT,
  NAME,
  BINARYADD,
  ASSIGN,
  FUNCTIONDEF,
  RETURN
} NodeType;

typedef struct Constant {
  int value; // TODO: pointer
} Constant;

typedef struct Name {
  char *id;
} Name;

typedef struct BinaryAdd {
  struct Node *left;
  struct Node *right;
} BinaryAdd;

typedef struct Assign {
  Name *target;
  struct Node *value;
} Assign;

typedef struct Return {
  struct Node *value; 
} Return;

typedef struct FunctionDef {
  char *name;
  char *args[5]; // NOTE: limit to 5 arguments
  struct Node *ret;
} FunctionDef;

typedef struct Node {
  NodeType type;
  union {
    Constant *constant;
    Name *name;
    BinaryAdd *binary_add;
    Assign *assign;
    FunctionDef *function_def;
    Return *ret;
  } data;
} Node;

typedef struct Module {
  Node *nodes[20];
} Module;

void module_print(Module *m);
void module_walk(Module *m, char **output, int *offset);
Module *parse(const char *input); // main entry point

#endif
