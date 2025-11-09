#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"

Token *tokenize(const char *source) {
  // malloc fixed array of tokens
  Token *tok = malloc(100 * sizeof(Token));
  int t_idx = 0; // token idx
  char buf[64]; // buffer for lexeme
  int b_idx = 0; // buffer idx

  int i; // character index
  int c; // character being scanned 
  while ((c = source[i]) != '\0') {
    if (isdigit(c)) {
      // start accumulating
      while (isdigit(c)) {
        buf[b_idx++] = c;
        c = source[++i]; 
      }
      // when done, emit T_INT and clear buffer
      // NOTE: no malloc needed, the structs already exist in memory uninit'd
      tok[t_idx].type = T_INT;
      buf[b_idx++] = '\0';
      tok[t_idx].lexeme = malloc(b_idx);
      sprintf(tok[t_idx++].lexeme, "%s", buf);
      // reset buffer
      b_idx = 0;
    } else if (isalpha(c) || c == '_') {
      // start accumulating name
      while (isalnum(c) || c == '_') {
        buf[b_idx++] = c;
        c = source[++i];
      }

      // check keywords
      int is_keyword = 0;
      Keyword keyword;
      for (int j = 0; j < num_keywords; j++) {
        keyword = keywords[j];
        if (strncmp(buf, keyword.kw, keyword.length) == 0) {
          tok[t_idx++].type = keyword.type;
          is_keyword = 1;
        }
      }

      // otherwise it's a normal identifier
      if (is_keyword == 0) {
        // else emit name with lexeme
        tok[t_idx].type = T_NAME;
        buf[b_idx++] = '\0';
        tok[t_idx].lexeme = malloc(b_idx);
        sprintf(tok[t_idx++].lexeme, "%s", buf);
      }

      // reset buffer
      b_idx = 0;

    } else if (c == ' ') {
      i++;
    } else if (c == '+') { // operators
      tok[t_idx++].type = T_PLUS;
      i++;
    } else if (c == '-') {
      tok[t_idx++].type = T_MINUS;
      i++;
    } else if (c == '*') {
      tok[t_idx++].type = T_MULTIPLY;
      i++;
    } else if (c == '/') {
      tok[t_idx++].type = T_DIVIDE;
      i++;
    } else if (c == '=') {
      if (source[i+1] == '=') {
        tok[t_idx++].type = T_EQ;
        i += 2;
      } else {
        tok[t_idx++].type = T_ASSIGN;
        i += 1;
      }
    } else if (c == '>') {
      if (source[i+1] == '=') {
        tok[t_idx++].type = T_GEQ;
        i += 2;
      } else {
        tok[t_idx++].type = T_GT;
        i += 1;
      }
    } else if (c == '<') {
      if (source[i+1] == '=') {
        tok[t_idx++].type = T_LEQ;
        i += 2;
      } else {
        tok[t_idx++].type = T_LT;
        i += 1;
      }
    } else if (c == '(') { // punctuation + grouping
      tok[t_idx++].type = T_LPAREN;
      i++;
    } else if (c == ')') {
      tok[t_idx++].type = T_RPAREN;
      i++;
    } else if (c == ',') {
      tok[t_idx++].type = T_COMMA;
      i++;
    } else if (c == ':') {
      tok[t_idx++].type = T_COLON;
      i++;
    } else if (c == '\n') {
      tok[t_idx++].type = T_NEWLINE;
      i++;
    } else {
      printf("error: we do not handle non-integers yet!\n");
      exit(1);
    }
  }
  
  tok[t_idx].type = T_EOF;
  return tok;
}

int main() {
  // try to tokenize some basic code 
  Token *tokens = tokenize("foo = (2 + 3) * 5\nbar = 4 + 5 - foo\n");
  Token t;
  for (int i=0; ((t = tokens[i]).type) != T_EOF; i++) {
    if (t.lexeme != NULL) {
      printf("%s, %s\n", token_table[t.type], t.lexeme);
    } else {
      printf("%s\n", token_table[t.type]);
    }
  }
}
