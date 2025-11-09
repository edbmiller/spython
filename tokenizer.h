#ifndef TOKEN_H
#define TOKEN_H

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

#endif
