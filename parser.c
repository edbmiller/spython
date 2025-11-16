#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parser.h"

Token *tokenize(const char *source) {
  // malloc fixed array of tokens
  Token *tok = malloc(100 * sizeof(Token));
  int t_idx = 0; // token idx
  char buf[64]; // buffer for lexeme
  int b_idx = 0; // buffer idx

  int i; // character index
  int c; // character being scanned 
  int level = 0; // indentation level
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
      strcpy(tok[t_idx++].lexeme, buf);
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
        strcpy(tok[t_idx++].lexeme, buf);
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
      // count leading spaces + check indentation
      int num_leading_spaces = 0;
      int gap_size; // between num_leading_spaces and level*4
      while (source[i] == ' ') {
        num_leading_spaces++; 
        i++;
      }
      if (num_leading_spaces == (level + 1) * 4) {
        tok[t_idx++].type = T_INDENT;
        level++;
      } else if ((gap_size = (num_leading_spaces - (level * 4))) % 4 == 0) {
        // count DEDENTs - note: 0 if no spaces :)
        for (int k=0; k>gap_size/4; k--) {
          tok[t_idx++].type = T_DEDENT;
        } 
      } else {
        printf("IndentationError\n");
        exit(1);
      }
    } else {
      printf("error: we do not handle non-integers yet!\n");
      exit(1);
    }
  }
 
  tok[t_idx].type = T_EOF;
  return tok;
}

static const char *SYNTAX_ERROR_MESSAGE = "SyntaxError: invalid syntax\n";

void assert_token_type_equals(TokenType a, TokenType b, const char *error) {
  if (a != b) {
    printf("%s", error);
    exit(1);
  }
}

void expect(TokenType a, TokenType b) {
  assert_token_type_equals(a, b, SYNTAX_ERROR_MESSAGE);
}

void expect_in(TokenType r, TokenType group[], int group_length) {
  for (int i=0; i<group_length; i++) {
    if (r == group[i]) {
      return;
    }
  }

  printf("%s", SYNTAX_ERROR_MESSAGE);
  exit(1);
}

Node *parse_expression(const Token *tokens, int *t_idx) {
  // every expression is OPERAND -> OPERATOR -> OPERAND -> OPERATOR -> OPERAND -> ...
  // ending in an OPERAND - no parentheses except in function calls
  // NOTE: support names, literals and binary operators - NOT functions yet!
  Node *result = malloc(sizeof(Node));
  TokenType operand_token_group[2] = { T_INT, T_NAME };
  TokenType operator_token_group[2] = { T_PLUS, T_MINUS };

  // start expecting stuff
  expect_in(tokens[*t_idx].type, operand_token_group, 2);
  Node *left_node = malloc(sizeof(Node));
  if (tokens[*t_idx].type == T_INT) {
    Constant *c = malloc(sizeof(Constant)); 
    c->value = atoi(tokens[*t_idx].lexeme);
    left_node->type = CONSTANT;
    left_node->data.constant = c;
  } else if (tokens[*t_idx].type == T_NAME) {
    Name *n = malloc(sizeof(Name));
    n->id = tokens[*t_idx].lexeme;
    left_node->type = NAME;
    left_node->data.name = n;
  }
  if (tokens[++(*t_idx)].type == T_NEWLINE) {
    (*t_idx)++;
    return left_node;
  } else if (tokens[*t_idx].type == T_EOF) {
    return left_node;
  } else {
    expect_in(tokens[*t_idx].type, operator_token_group, 2);
    expect(tokens[*t_idx].type, T_PLUS); // TODO: support other ops
    BinaryAdd *b = malloc(sizeof(BinaryAdd)); 
    b->left = left_node;
    // bump past operator and parse rest of expression 
    (*t_idx)++;
    b->right = parse_expression(tokens, t_idx);
    // now malloc result node and return
    Node *binary_add_node = malloc(sizeof(Node));
    binary_add_node->type = BINARYADD;
    binary_add_node->data.binary_add = b;
    // (*t_idx)++; // damn this has already happened when he hit the newline
    return binary_add_node;
  }
}

Module *parse(const Token *tokens) {
  Module *result = malloc(sizeof(Module));
  int n_idx = 0; // node index
  int t_idx = 0; // token index
  while (tokens[t_idx].type != T_EOF) {
    if (tokens[t_idx].type == T_DEF) {
      FunctionDef *f = malloc(sizeof(FunctionDef));
      // expect name and allocate it
      expect(tokens[++t_idx].type, T_NAME);
      f->name = malloc(10);
      strcpy(f->name, tokens[t_idx].lexeme);
      // expect (
      expect(tokens[++t_idx].type, T_LPAREN); 
      // accumulate argnames
      expect(tokens[++t_idx].type, T_NAME);
      // do first one
      f->args = malloc(5 * sizeof(char *)); 
      f->args[0] = malloc(10);
      strcpy(f->args[0], tokens[t_idx].lexeme);
      // do rest
      int a_idx = 1;
      while (tokens[++t_idx].type == T_COMMA) {
        expect(tokens[++t_idx].type, T_NAME);
        f->args[a_idx] = malloc(10);
        strcpy(f->args[a_idx], tokens[t_idx].lexeme);
        a_idx++;
      }
      expect(tokens[t_idx].type, T_RPAREN);
      expect(tokens[++t_idx].type, T_COLON);
      expect(tokens[++t_idx].type, T_NEWLINE);
      expect(tokens[++t_idx].type, T_INDENT);
      t_idx++;
      // TODO: parse until DEDENT'd out of block
      parse_expression(tokens, &t_idx);
    } else if (tokens[t_idx].type == T_RETURN) {
      Return *r = malloc(sizeof(Return));
      t_idx++;
      r->value = parse_expression(tokens, &t_idx);
      Node *ret_node = malloc(sizeof(Node));
      ret_node->type = RETURN;
      ret_node->data.ret = r;
      result->nodes[n_idx++] = ret_node;
      break;
    } else if (tokens[t_idx].type == T_IF) {
      ;
    } else if (tokens[t_idx].type == T_ELSE) {
      ;
    } else if (tokens[t_idx].type == T_NAME && tokens[t_idx+1].type == T_ASSIGN) {
      // assignment
      Assign *ass = malloc(sizeof(Assign));
      Name *name = malloc(sizeof(Name));
      name->id = tokens[t_idx].lexeme;
      ass->target = name;
      t_idx += 2;
      ass->value = parse_expression(tokens, &t_idx);
      Node *ass_node = malloc(sizeof(Node));
      ass_node->type = ASSIGN;
      ass_node->data.assign = ass;
      result->nodes[n_idx++] = ass_node; 
    } else {
      result->nodes[n_idx++] = parse_expression(tokens, &t_idx);
    }
  }
  return result;
}

char *space(int n) {
  char *buf = malloc(n+1);
  for (int i=0; i<n; i++) {
    buf[i] = ' ';
  }
  buf[n] = '\0';
  return buf;
}

// NOTE: first do non-compound nodes
char *node_format(Node *n, int indent) {
  char *result = malloc(500); 
  char *start = result;
  if (n->type == CONSTANT) {
    result += sprintf(result, "Constant(value=%d)", n->data.constant->value);
  } else if (n->type == NAME) {
    result += sprintf(result, "Name(id='%s')", n->data.name->id);
  } else if (n->type == BINARYADD) {
    result += sprintf(
      result, 
      "BinOp(\n%sleft=%s,\n%sop=Add,\n%sright=%s\n%s)", 
      space(indent+2),
      node_format(n->data.binary_add->left, indent+2), 
      space(indent+2),
      space(indent+2),
      node_format(n->data.binary_add->right, indent+2),
      space(indent)
    );
  } else if (n->type == ASSIGN) {
    result += sprintf(
      result,
      "Assign(\n%starget=Name(id='%s'),\n%svalue=%s\n%s)",
      space(indent+2),
      n->data.assign->target->id,
      space(indent+2),
      node_format(n->data.assign->value, indent+2),
      space(indent)
    );
  } else if (n->type == RETURN) {
    result += sprintf(
      result,
      "Return(\n%svalue=%s\n%s)",
      space(indent+2),
      node_format(n->data.ret->value, indent+2),
      space(indent)
    );
  }
  return start;
}

void module_print(Module *m) {
  int level = 0;
  Node *n;
  for (int i=0; ((n=m->nodes[i])) != NULL; i++) {
    printf("%s\n", node_format(n, 0));
  }
}

int main() {
  // tokenize some basic code
  Token *tokens = tokenize("a = 1\nfoo = 2 + a\nreturn foo");
  Token t;
  printf("tokens = \n");
  for (int i=0; ((t = tokens[i]).type) != T_EOF; i++) {
    if (t.lexeme != NULL) {
      printf("%s, %s\n", token_table[t.type], t.lexeme);
    } else {
      printf("%s\n", token_table[t.type]);
    }
  }
  printf("\n");

  Module *module = parse(tokens);
  printf("module = \n");
  module_print(module);
}
