#ifndef PARSER_H
#define PARSER_H

#include "hash-table.h"

// NOTE: define our nodes
typedef enum NodeType {
  CONSTANT,
  NAME,
  BINARYADD,
  ASSIGN,
  FUNCTIONDEF,
  RETURN,
  CALLFUNCTION,
  EXPR,
  IF,
  COMPARE
} NodeType;

static char *node_type_table[9] = {
  "CONSTANT",
  "NAME",
  "BINARYADD",
  "ASSIGN",
  "FUNCTIONDEF",
  "RETURN",
  "CALLFUNCTION",
  "EXPR",
  "IF"
};

// -------- BINARY COMPARISON OP BOILERPLATE
typedef enum {
  EQUALS = 0,
  GREATER_THAN,
  LESS_THAN
} Comparison;

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
  char **args; // NOTE: limit to 5 arguments
  struct Module *body;
} FunctionDef;

typedef struct CallFunction {
  Name *func;
  struct Node **args;
} CallFunction;

// NOTE: pure expressions e.g. `\n3 + 4` or `\nprint()`
typedef struct Expr {
  struct Node *value;
} Expr;

typedef struct If {
  struct Node *test;
  struct Module *body;
  struct Module *orelse;
} If;

typedef struct Compare {
  struct Node *left;
  struct Node *right;
  int comparison;
} Compare;

typedef struct Node {
  NodeType type;
  union {
    Constant *constant;
    Name *name;
    BinaryAdd *binary_add;
    Assign *assign;
    FunctionDef *function_def;
    Return *ret;
    CallFunction *call_function;
    Expr *expr;
    If *iff;
    Compare *compare;
  } data;
} Node;

typedef struct Module {
  Node *nodes[20];
} Module;

void module_print(Module *m);
PyCodeObject *module_walk(Module *m);
Module *parse(const char *input, int indentation_depth, int *distance_travelled); // main entry point

#endif
