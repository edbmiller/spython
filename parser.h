#ifndef TOKEN_H
#define TOKEN_H

#include "hash-table.h"

// tokenizer stuff ----------------------------
typedef enum {
  // literals + identifiers
  T_INT,
  T_NAME,

  // keywords
  T_DEF,
  T_RETURN,
  T_IF,
  T_ELSE,
  
  // operators
  T_PLUS,
  T_MINUS,
  T_ASSIGN,
  T_MULTIPLY,
  T_DIVIDE,
  T_EQ,
  T_LT,
  T_GT,
  T_LEQ,
  T_GEQ,

  // punctuation + grouping 
  T_LPAREN,
  T_RPAREN,
  T_COMMA,
  T_COLON,
  T_NEWLINE,
  T_INDENT,
  T_DEDENT,
  
  // end of file
  T_EOF
} TokenType;

static char *token_table[24] = {
  "INT",
  "NAME",
  "DEF",
  "RETURN",
  "IF",
  "ELSE",
  "PLUS",
  "MINUS",
  "ASSIGN",
  "MULTIPLY",
  "DIVIDE",
  "EQ",
  "LT",
  "GT",
  "LEQ",
  "GEQ",
  "LPAREN",
  "RPAREN",
  "COMMA",
  "COLON",
  "NEWLINE",
  "INDENT",
  "DEDENT",
  "EOF"
};

typedef struct {
  const char *kw;
  int length;
  TokenType type;
} Keyword;

static const int num_keywords = 4;

static Keyword keywords[num_keywords] = {
  { "def", 3, T_DEF },
  { "return", 6, T_RETURN },
  { "if", 2, T_IF },
  { "else", 4, T_ELSE },
};

// e.g. Token{type: T_NAME, lexeme: "foo"}
typedef struct {
  TokenType type;
  char *lexeme;
} Token;

Token *tokenize(const char *source);

// parser stuff -------------------------------
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

typedef struct Expr { // NOTE: pure expressions e.g. `3 + 4`
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
Module *parse(const Token *tokens); // main entry point

#endif
