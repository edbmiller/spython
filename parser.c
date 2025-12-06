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

  int i = 0; // character index
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
      tok[t_idx].type = T_PLUS;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == '-') {
      tok[t_idx++].type = T_MINUS;
      tok[t_idx].lexeme = NULL;
      i++;
    } else if (c == '*') {
      tok[t_idx].type = T_MULTIPLY;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == '/') {
      tok[t_idx].type = T_DIVIDE;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == '=') {
      if (source[i+1] == '=') {
        tok[t_idx].type = T_EQ;
        tok[t_idx++].lexeme = NULL;
        i += 2;
      } else {
        tok[t_idx].type = T_ASSIGN;
        tok[t_idx++].lexeme = NULL;
        i += 1;
      }
    } else if (c == '>') {
      if (source[i+1] == '=') {
        tok[t_idx].type = T_GEQ;
        tok[t_idx++].lexeme = NULL;
        i += 2;
      } else {
        tok[t_idx].type = T_GT;
        tok[t_idx++].lexeme = NULL;
        i += 1;
      }
    } else if (c == '<') {
      if (source[i+1] == '=') {
        tok[t_idx].type = T_LEQ;
        tok[t_idx++].lexeme = NULL;
        i += 2;
      } else {
        tok[t_idx].type = T_LT;
        tok[t_idx++].lexeme = NULL;
        i += 1;
      }
    } else if (c == '(') { // punctuation + grouping
      tok[t_idx].type = T_LPAREN;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == ')') {
      tok[t_idx].type = T_RPAREN;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == ',') {
      tok[t_idx].type = T_COMMA;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == ':') {
      tok[t_idx].type = T_COLON;
      tok[t_idx++].lexeme = NULL;
      i++;
    } else if (c == '\n') {
      tok[t_idx].type = T_NEWLINE;
      tok[t_idx++].lexeme = NULL;
      i++;
      // count leading spaces + check indentation
      int num_leading_spaces = 0;
      int gap_size; // between num_leading_spaces and level*4
      while (source[i] == ' ') {
        num_leading_spaces++; 
        i++;
      }
      if (num_leading_spaces == (level + 1) * 4) {
        tok[t_idx].type = T_INDENT;
        tok[t_idx++].lexeme = NULL,
        level++;
      } else if ((gap_size = (num_leading_spaces - (level * 4))) % 4 == 0) {
        // count DEDENTs - note: 0 if no spaces :)
        for (int k=0; k>gap_size/4; k--) {
          tok[t_idx].type = T_DEDENT;
          tok[t_idx++].lexeme = NULL;
          level--;
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
  tok[t_idx].lexeme = NULL;
  return tok;
}

static const char *SYNTAX_ERROR_MESSAGE = "SyntaxError: invalid syntax";

void assert_token_type_equals(TokenType a, TokenType b, const char *error) {
  if (a != b) {
    printf("%s", SYNTAX_ERROR_MESSAGE);
    printf(" - expected %s, saw %s\n", token_table[b], token_table[a]);
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
  printf(" - expected group %s, saw %s\n", token_table[group[0]], token_table[r]);
  exit(1);
}

int is_in(TokenType r, TokenType group[], int group_length) {
  for (int i=0; i<group_length; i++) {
    if (r == group[i]) {
      return 0;
    }
  }
  return 1;
}

Node *parse_flat_expression(const Token *tokens, int *t_idx, const Node *nodes) {
  // parse rest left-to-right
  Node *result = malloc(sizeof(Node));
  TokenType operand_token_group[3] = { T_INT, T_NAME, T_NODE };
  TokenType operator_token_group[2] = { T_PLUS, T_MINUS };

  // start expecting stuff
  expect_in(tokens[*t_idx].type, operand_token_group, 3);
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
  } else if (tokens[*t_idx].type == T_NODE) {
    int node_idx = atoi(tokens[*t_idx].lexeme);
    left_node = &nodes[node_idx];
  }
  
  if (tokens[*t_idx+1].type == T_NEWLINE) {
    *t_idx += 2;
    return left_node;
  } else if (tokens[*t_idx+1].type == T_EOF) {
    *t_idx += 1;
    return left_node;
  } else {
    (*t_idx)++; // bump forward to next token
    if (is_in(tokens[*t_idx].type, operator_token_group, 2) == 0) {
      expect(tokens[*t_idx].type, T_PLUS);
      BinaryAdd *b = malloc(sizeof(BinaryAdd)); 
      b->left = left_node;
      // bump past operator and parse rest of expression 
      (*t_idx)++;
      b->right = parse_flat_expression(tokens, t_idx, nodes);
      // now malloc result node and return
      Node *binary_add_node = malloc(sizeof(Node));
      binary_add_node->type = BINARYADD;
      binary_add_node->data.binary_add = b;
      return binary_add_node;
    } else {
      // TODO: make this not a fuckery...
      return left_node;
    }
  }
}

Node *parse_expression(const Token *tokens, int *t_idx) {
  // build nodes as we go and replace sub-expressions
  // with T_NODE,<idx> as we do them
  Node *nodes = malloc(10 * sizeof(Node));
  int n_idx = 0;

  // get length
  int length = 0;
  while (tokens[length].type != T_EOF) {
    length++;
  }
 
  // TODO: subsequent passes with lower precedence operators
  // first pass: function calls
  Token *new_tokens = malloc((length+1) * sizeof(Token));
  int new_t_idx = 0;
  while (tokens[*t_idx].type != T_EOF && tokens[*t_idx].type != T_COMMA && tokens[*t_idx].type != T_RPAREN) {
    if (tokens[*t_idx].type == T_NAME && tokens[*t_idx+1].type == T_LPAREN) {
      // allocate func name
      Name *n = malloc(sizeof(Name));
      n->id = tokens[*t_idx].lexeme;

      // allocate call 
      CallFunction *call = malloc(sizeof(CallFunction));
      call->func = n;

      // move fwd + allocate args
      *t_idx += 2;
      int arg_idx = 0;
      call->args = malloc(5 * sizeof(Node));
      while (tokens[*t_idx].type != T_RPAREN) {
        // TODO: perhaps return Node, not Node* ???
        Node *arg_node = parse_expression(tokens, t_idx);
        call->args[arg_idx] = *arg_node;
        arg_idx++;
        if (tokens[*t_idx].type == T_COMMA) {
          (*t_idx)++;
        } else if (tokens[*t_idx].type == T_RPAREN) {
          (*t_idx)++;
          break;
        } else {
          printf("%s", SYNTAX_ERROR_MESSAGE);
          printf("\nbad func\n");
          exit(1);
        }
      }
      call->argc = arg_idx;
      Node *call_node = malloc(sizeof(Node));
      call_node->type = CALLFUNCTION;
      call_node->data.call_function = call;

      // now we've allocated the function call node
      // => insert into node array and emit T_NODE
      nodes[n_idx] = *call_node;

      // build new T_NODE token
      Token new_token;
      new_token.type = T_NODE;
      new_token.lexeme = malloc(12);
      sprintf(new_token.lexeme, "%d", n_idx++);
      // printf("new token lexeme: %s\n", new_token.lexeme);
      // printf("references node of type %s", node_type_table[nodes[n_idx-1].type]);

      // emit it
      new_tokens[new_t_idx++] = new_token;
    } else {
      // otherwise just emit the same token
      new_tokens[new_t_idx++] = tokens[(*t_idx)++];
    }
  }

  // finally terminate with T_EOF by hand
  new_tokens[new_t_idx].type = T_EOF;
  new_t_idx = 0;
  Node *result = parse_flat_expression(new_tokens, &new_t_idx, nodes);
  return result;
}

Module *parse(const Token *tokens, int *t_idx) {
  Module *result = malloc(sizeof(Module));
  int n_idx = 0; // node index
  while (tokens[*t_idx].type != T_EOF) {
    if (tokens[*t_idx].type == T_DEF) {
      FunctionDef *f = malloc(sizeof(FunctionDef));
      // expect name and allocate it
      expect(tokens[++(*t_idx)].type, T_NAME);
      f->name = malloc(10);
      strcpy(f->name, tokens[*t_idx].lexeme);
      // expect (
      expect(tokens[++(*t_idx)].type, T_LPAREN); 
      // accumulate argnames
      expect(tokens[++(*t_idx)].type, T_NAME);
      // do first one
      f->args = malloc(5 * sizeof(char *)); 
      f->args[0] = malloc(10);
      strcpy(f->args[0], tokens[*t_idx].lexeme);
      // do rest
      int a_idx = 1;
      while (tokens[++(*t_idx)].type == T_COMMA) {
        expect(tokens[++(*t_idx)].type, T_NAME);
        f->args[a_idx] = malloc(10);
        strcpy(f->args[a_idx], tokens[*t_idx].lexeme);
        a_idx++;
      }
      expect(tokens[*t_idx].type, T_RPAREN);
      expect(tokens[++(*t_idx)].type, T_COLON);
      expect(tokens[++(*t_idx)].type, T_NEWLINE);
      expect(tokens[++(*t_idx)].type, T_INDENT);
      (*t_idx)++;
      f->body = parse(tokens, t_idx);
      if (tokens[*t_idx].type != T_EOF) {
        expect(tokens[(*t_idx)++].type, T_DEDENT);
      }
      Node *f_node = malloc(sizeof(Node));
      f_node->type = FUNCTIONDEF;
      f_node->data.function_def = f;
      result->nodes[n_idx++] = f_node;
    } else if (tokens[*t_idx].type == T_RETURN) {
      Return *r = malloc(sizeof(Return));
      (*t_idx)++;
      r->value = parse_expression(tokens, t_idx);
      Node *ret_node = malloc(sizeof(Node));
      ret_node->type = RETURN;
      ret_node->data.ret = r;
      result->nodes[n_idx++] = ret_node;
      break;
    } else if (tokens[*t_idx].type == T_IF) {
      // Node *test_node = parse_expression(...)
      ; 
    } else if (tokens[*t_idx].type == T_ELSE) {
      ;
    } else if (tokens[*t_idx].type == T_NAME && tokens[*t_idx+1].type == T_ASSIGN) {
      // assignment
      Assign *ass = malloc(sizeof(Assign));
      Name *name = malloc(sizeof(Name));
      name->id = tokens[*t_idx].lexeme;
      ass->target = name;
      *t_idx += 2;
      ass->value = parse_expression(tokens, t_idx);
      Node *ass_node = malloc(sizeof(Node));
      ass_node->type = ASSIGN;
      ass_node->data.assign = ass;
      result->nodes[n_idx++] = ass_node; 
    } else if (tokens[*t_idx].type == T_NEWLINE) {
      // this is a pure newline we see outside of
      // an expression or terminating a block
      (*t_idx)++;
    } else {
      result->nodes[n_idx++] = parse_expression(tokens, t_idx);
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
      "%sAssign(\n%starget=Name(id='%s'),\n%svalue=%s\n%s)",
      space(indent),
      space(indent+2),
      n->data.assign->target->id,
      space(indent+2),
      node_format(n->data.assign->value, indent+2),
      space(indent)
    );
  } else if (n->type == RETURN) {
    result += sprintf(
      result,
      "%sReturn(\n%svalue=%s\n%s)",
      space(indent),
      space(indent+2),
      node_format(n->data.ret->value, indent+2),
      space(indent)
    );
  } else if (n->type == CALLFUNCTION) {
    result += sprintf(
      result,
      "Call(\n%sfunc=Name(id='%s'),\n%sargs=[\n",
      space(indent+2),
      n->data.call_function->func->id,
      space(indent+2)
    );
    for (int i=0; i<n->data.call_function->argc; i++) {
      result += sprintf(result, 
        "%s%s,\n", 
        space(indent+4), 
        node_format(n->data.call_function->args+i, indent+4)
      );
    }
    result += sprintf(result, "%s]\n%s)", space(indent+2), space(indent));
  } else if (n->type == FUNCTIONDEF) {
    result += sprintf(
      result,
      "FunctionDef(\n%sname='%s',\n%sargs=[",
      space(indent+2),
      n->data.function_def->name,
      space(indent+2)
    );
    for (int i=0; n->data.function_def->args[i] != NULL; i++) {
      result += sprintf(
        result,
        "%s,", 
        n->data.function_def->args[i]
      );
    }
    result += sprintf(
      result,
      "],\n%sbody=[\n",
      space(indent+2)
    );
    for (int j=0; n->data.function_def->body->nodes[j] != NULL; j++) {
      result += sprintf(
        result,
        node_format(n->data.function_def->body->nodes[j], indent+4)
      );
      if (n->data.function_def->body->nodes[j+1] != NULL) {
        result += sprintf(result, ",\n");
      } else {
        result += sprintf(
          result, 
          "\n%s]\n%s)", 
          space(indent+2),
          space(indent)
        );
      }
    }
  }
  return start;
}

void module_print(Module *m) {
  Node *n;
  for (int i=0; ((n=m->nodes[i])) != NULL; i++) {
    printf("%s\n", node_format(n, 0));
  }
  printf("\n");
}

void walk(Node *node, char **bytecode, int *b_idx, PyObject **consts, int *c_idx) {
  // post-order traverse AST and emit bytecode to output
  // buffer according to the current offset
  // first: allocate string
  switch (node->type) {
    case CONSTANT:
      PyIntObject *constant = malloc(sizeof(PyIntObject));
      constant->type = PY_INT;
      constant->value = node->data.constant->value;
      consts[*c_idx] = (PyObject *) constant;
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_CONST,%d", *c_idx);
      *b_idx += 1;
      *c_idx += 1;
      break;
    case NAME:
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_NAME,'%s'", node->data.name->id);
      *b_idx += 1;
      break;
    case BINARYADD:
      walk(node->data.binary_add->left, bytecode, b_idx, consts, c_idx);
      walk(node->data.binary_add->right, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = "ADD";
      *b_idx += 1;
      break;
    case ASSIGN:
      walk(node->data.assign->value, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "STORE_NAME,'%s'", node->data.assign->target->id);
      *b_idx += 1;
      break;
    case FUNCTIONDEF:
      // 1. build PyCodeObject
      PyCodeObject *code = module_walk(node->data.function_def->body);
      code->argnames = node->data.function_def->args;

      // 2. save it to consts and emit a LOAD_CONST
      consts[*c_idx] = (PyObject *) code;
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_CONST,%d", *c_idx);
      *c_idx += 1;
      *b_idx += 1;

      // 3. emit MAKE_FUNCTION and STORE_NAME
      bytecode[*b_idx] = "MAKE_FUNCTION";
      *b_idx += 1;
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "STORE_NAME,'%s'", node->data.function_def->name);
      *b_idx += 1;
      break;
    case RETURN:
      walk(node->data.ret->value, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = "RETURN";
      *b_idx += 1;
      break;
    case CALLFUNCTION:
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_NAME,'%s'", node->data.call_function->func->id);
      *b_idx += 1;
      // for each argument, emit a LOAD_ opcode
      int i = 0;
      while (i < node->data.call_function->argc) {
        // walk the arg
        walk(node->data.call_function->args+i, bytecode, b_idx, consts, c_idx);
        i++;
      }
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "CALL_FUNCTION,%d", i);
      *b_idx += 1;
      break;
    case EXPR:
      // walk the expression then pop the result
      walk(node->data.expr->value, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = "POP_TOP";
      *b_idx += 1;
      break;
    case IF:
      walk(node->data.iff->test, bytecode, b_idx, consts, c_idx);
      int pop_jump_offset = *b_idx; // patch when we know block sizes
      *b_idx += 1;
      for (int j=0; node->data.iff->body->nodes[j] != NULL; j++) {
        walk(node->data.iff->body->nodes[j], bytecode, b_idx, consts, c_idx);
      }
      if (node->data.iff->orelse != NULL) {
        // put the extra JUMP after true block and patch POP_JUMP_IF_FALSE
        int extra_jump_offset = *b_idx;
        *b_idx += 1;
        bytecode[pop_jump_offset] = malloc(32);
        sprintf(bytecode[pop_jump_offset], "POP_JUMP_IF_FALSE,%d", *b_idx);
        // now walk the orelse
        for (int j=0; node->data.iff->orelse->nodes[j] != NULL; j++) {
          walk(node->data.iff->orelse->nodes[j], bytecode, b_idx, consts, c_idx);
        }
        // finally patch the jump to skip if we've done the true block
        bytecode[extra_jump_offset] = malloc(32);
        sprintf(bytecode[extra_jump_offset], "JUMP,%d", *b_idx);
      } else {
        bytecode[pop_jump_offset] = malloc(32);
        sprintf(bytecode[pop_jump_offset], "POP_JUMP_IF_FALSE,%d", *b_idx);
        // nb. *b_idx has been incr'd by body walk
      }
      break;
    case COMPARE:
      walk(node->data.compare->left, bytecode, b_idx, consts, c_idx);
      walk(node->data.compare->right, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "COMPARE,%d", node->data.compare->comparison);
      *b_idx += 1;
      break;
  }
}

PyCodeObject *module_walk(Module *module) {
  PyCodeObject *result = malloc(sizeof(PyCodeObject));
  result->type = PY_CODE;
  result->bytecode = malloc(100 * sizeof(char *));
  result->consts = malloc(10 * sizeof(PyObject *));
  result->argnames = malloc(5 * sizeof(char *));
  int *bytecode_offset = malloc(sizeof(int));
  int *consts_offset = malloc(sizeof(int));
  for (int i=0; module->nodes[i] != NULL; i++) {
    walk(module->nodes[i], result->bytecode, bytecode_offset, result->consts, consts_offset);
  }
  return result;
}

void print_tokens(Token *tokens) {
  Token t;
  printf("tokens = \n");
  for (int i=0; ((t = tokens[i]).type) != T_EOF; i++) {
    if (t.lexeme != NULL) {
      printf("%d: %s, %s\n", i, token_table[t.type], t.lexeme);
    } else {
      printf("%d: %s\n", i, token_table[t.type]);
    }
  }
  printf("\n");
}

int main() {
  Token *tokens = tokenize("2 + foo(3, foo(3))");
  print_tokens(tokens);

  int t_idx = 0;
  Module *module = parse(tokens, &t_idx);
  printf("module = \n");
  module_print(module);

  printf("bytecode = \n");
  PyCodeObject *code = module_walk(module);
  for (int i=0; code->bytecode[i] != NULL; i++) {
    printf("%s\n", code->bytecode[i]);
  }
}
