#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "parser.h"

void token_array_init(TokenArray *a) {
  const int INITIAL_SIZE = 8;
  a->count = 0;
  a->size = INITIAL_SIZE;
  a->data = malloc(INITIAL_SIZE * sizeof(Token));
}

void token_array_push(TokenArray *a, Token t) {
  if (a->count == a->size) {
    // realloc
    size_t new_size = 2 * a->size;
    a->data = realloc(a->data, new_size * sizeof(Token));
    a->size = new_size;
  }
  // now push
  a->data[(a->count)++] = t;
}

// TODO: module vs node array lingo
TokenArray tokenize(const char *source) {
  // init token array
  TokenArray tokens;
  token_array_init(&tokens); 

  char buf[64]; // buffer for lexeme
  int b_idx = 0; // buffer idx

  int i = 0; // character index
  int c; // character being scanned 
  int level = 0; // indentation level
  while ((c = source[i]) != '\0') {
    Token token; // to be added
    if (isdigit(c)) {
      // start accumulating
      while (isdigit(c)) {
        buf[b_idx++] = c;
        c = source[++i]; 
      }
      buf[b_idx++] = '\0';
      // when done, emit T_INT and clear buffer
      token.type = T_INT;
      token.lexeme = malloc(b_idx);
      strcpy(token.lexeme, buf);
      token_array_push(&tokens, token);
      // reset buffer
      b_idx = 0;
    } else if (isalpha(c) || c == '_') {
      // start accumulating name
      while (isalnum(c) || c == '_') {
        buf[b_idx++] = c;
        c = source[++i];
      }
      buf[b_idx++] = '\0';

      // check keywords
      int is_keyword = 0;
      Keyword keyword;
      for (int j = 0; j < num_keywords; j++) {
        keyword = keywords[j];
        if (strncmp(buf, keyword.kw, keyword.length) == 0) {
          token.type = keyword.type;
          token.lexeme = NULL;
          token_array_push(&tokens, token);
          is_keyword = 1;
        }
      }

      // otherwise it's a normal identifier
      if (is_keyword == 0) {
        // else emit name with lexeme
        token.type = T_NAME;
        token.lexeme = malloc(b_idx);
        strcpy(token.lexeme, buf);
        token_array_push(&tokens, token);
      }

      // reset buffer
      b_idx = 0;
    } else if (c == ' ') {
      i++;
    } else if (c == '+') { // operators
      token.type = T_PLUS;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == '-') {
      token.type = T_MINUS;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == '*') {
      token.type = T_MULTIPLY;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == '/') {
      token.type = T_DIVIDE;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == '=') {
      if (source[i+1] == '=') {
        token.type = T_EQ;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 2;
      } else {
        token.type = T_ASSIGN;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 1;
      }
    } else if (c == '>') {
      if (source[i+1] == '=') {
        token.type = T_GEQ;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 2;
      } else {
        token.type = T_GT;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 1;
      }
    } else if (c == '<') {
      if (source[i+1] == '=') {
        token.type = T_LEQ;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 2;
      } else {
        token.type = T_LT;
        token.lexeme = NULL;
        token_array_push(&tokens, token);
        i += 1;
      }
    } else if (c == '(') { // punctuation + grouping
      token.type = T_LPAREN;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == ')') {
      token.type = T_RPAREN;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == ',') {
      token.type = T_COMMA;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == ':') {
      token.type = T_COLON;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
    } else if (c == '\n') {
      token.type = T_NEWLINE;
      token.lexeme = NULL;
      token_array_push(&tokens, token);
      i++;
      // count leading spaces + check indentation
      int num_leading_spaces = 0;
      int gap_size; // between num_leading_spaces and level*4
      while (source[i] == ' ') {
        num_leading_spaces++; 
        i++;
      }
      if (num_leading_spaces == (level + 1) * 4) {
        token.type = T_INDENT;
        token.lexeme = NULL,
        token_array_push(&tokens, token);
        level++;
      } else if ((gap_size = (num_leading_spaces - (level * 4))) % 4 == 0) {
        // count DEDENTs - note: 0 if no spaces :)
        for (int k=0; k>gap_size/4; k--) {
          token.type = T_DEDENT;
          token.lexeme = NULL;
          token_array_push(&tokens, token);
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

  Token final_token;
  final_token.type = T_EOF;
  final_token.lexeme = NULL;
  token_array_push(&tokens, final_token);
  return tokens;
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

typedef struct {
  Token *tokens;
  Node  *nodes;
  int    node_count;
} ParseResult;

ParseResult handle_functions(const Token *tokens, int *t_idx) {

  int length = 0;
  while (tokens[length].type != T_EOF) length++;

  Token *out_tokens = malloc((length + 1) * sizeof(Token));
  Node  *out_nodes  = malloc(10 * sizeof(Node));

  int out_t_idx = 0;
  int out_n_idx = 0;

  // TODO: think about terminate condition
  while (tokens[*t_idx].type != T_EOF && tokens[*t_idx].type != T_COMMA && tokens[*t_idx].type != T_RPAREN && tokens[*t_idx].type != T_COLON && tokens[*t_idx].type != T_NEWLINE) {
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
      out_nodes[out_n_idx] = *call_node;

      // build new T_NODE token
      Token new_token;
      new_token.type = T_NODE;
      new_token.lexeme = malloc(12);
      sprintf(new_token.lexeme, "%d", out_n_idx++);
      // printf("new token lexeme: %s\n", new_token.lexeme);
      // printf("references node of type %s", node_type_table[nodes[n_idx-1].type]);

      // emit it
      out_tokens[out_t_idx++] = new_token;
    } else {
      // otherwise just emit the same token
      out_tokens[out_t_idx++] = tokens[(*t_idx)++];
    }
  }
  out_tokens[out_t_idx].type = T_EOF;

  ParseResult result = {
    .tokens = out_tokens,
    .nodes = out_nodes,
    .node_count = out_n_idx,
  };
  return result;
}

ParseResult handle_multiply_divide(
    Token *tokens,
    Node *prev_nodes,
    int prev_node_count
) {
  int length = 0;
  while (tokens[length].type != T_EOF) length++;

  Token *out_tokens = malloc((length + 1) * sizeof(Token));
  Node  *out_nodes  = malloc(10 * sizeof(Node));

  int t_idx = 0;
  int out_t_idx = 0;
  int out_n_idx = 0;

  TokenType operand_group[3] = { T_INT, T_NAME, T_NODE };

  while (tokens[t_idx].type != T_EOF &&
         tokens[t_idx].type != T_COMMA &&
         tokens[t_idx].type != T_RPAREN &&
         tokens[t_idx].type != T_COLON &&
         tokens[t_idx].type != T_NEWLINE) {

    if (!(tokens[t_idx].type == T_INT || tokens[t_idx].type == T_NAME || tokens[t_idx].type == T_NODE)) {
      // emit and skip
      out_tokens[out_t_idx++] = tokens[t_idx++];
      continue;
    }

    /* ---- left operand ---- */
    Node *left;

    if (tokens[t_idx].type == T_NODE) {
      int idx = atoi(tokens[t_idx].lexeme);
      left = &prev_nodes[idx];
    } else {
      left = malloc(sizeof(Node));
      if (tokens[t_idx].type == T_INT) {
        Constant *c = malloc(sizeof(Constant));
        c->value = atoi(tokens[t_idx].lexeme);
        left->type = CONSTANT;
        left->data.constant = c;

      } else {
        Name *n = malloc(sizeof(Name));
        n->id = tokens[t_idx].lexeme;
        left->type = NAME;
        left->data.name = n;
      }
    }

    /* ---- fold * / chain ---- */
    while (tokens[t_idx + 1].type == T_MULTIPLY ||
           tokens[t_idx + 1].type == T_DIVIDE) {

      BinaryOp *bin = malloc(sizeof(BinaryOp));
      bin->op = (tokens[t_idx + 1].type == T_MULTIPLY) ? MULT : DIV;

      t_idx += 2;

      Node *right;
      if (tokens[t_idx].type == T_NODE) {
        int idx = atoi(tokens[t_idx].lexeme);
        right = &prev_nodes[idx];
      } else {
        right = malloc(sizeof(Node));

        if (tokens[t_idx].type == T_INT) {
          Constant *c = malloc(sizeof(Constant));
          c->value = atoi(tokens[t_idx].lexeme);
          right->type = CONSTANT;
          right->data.constant = c;

        } else {
          Name *n = malloc(sizeof(Name));
          n->id = tokens[t_idx].lexeme;
          right->type = NAME;
          right->data.name = n;
        }
      }

      bin->left = left;
      bin->right = right;

      Node *bin_node = malloc(sizeof(Node));
      bin_node->type = BINARYOP;
      bin_node->data.binary_op = bin;

      left = bin_node;
    }

    /* ---- emit node ---- */
    out_nodes[out_n_idx] = *left;

    Token t;
    t.type = T_NODE;
    t.lexeme = malloc(12);
    sprintf(t.lexeme, "%d", out_n_idx);

    out_tokens[out_t_idx++] = t;
    out_n_idx++;
    t_idx++;
  }

  out_tokens[out_t_idx].type = T_EOF;

  ParseResult result = {
    .tokens = out_tokens,
    .nodes = out_nodes,
    .node_count = out_n_idx
  };

  return result;
}

ParseResult handle_add_subtract(
    Token *tokens,
    Node *prev_nodes,
    int prev_node_count
) {
  int length = 0;
  while (tokens[length].type != T_EOF) length++;

  Token *out_tokens = malloc((length + 1) * sizeof(Token));
  Node  *out_nodes  = malloc(10 * sizeof(Node));

  int t_idx = 0;
  int out_t_idx = 0;
  int out_n_idx = 0;

  TokenType operand_group[3] = { T_INT, T_NAME, T_NODE };

  while (tokens[t_idx].type != T_EOF &&
         tokens[t_idx].type != T_COMMA &&
         tokens[t_idx].type != T_RPAREN &&
         tokens[t_idx].type != T_COLON &&
         tokens[t_idx].type != T_NEWLINE) {

    if (!(tokens[t_idx].type == T_INT || tokens[t_idx].type == T_NAME || tokens[t_idx].type == T_NODE)) {
      // emit and skip
      out_tokens[out_t_idx++] = tokens[t_idx++];
      continue;
    }

    /* ---- left operand ---- */
    Node *left;

    if (tokens[t_idx].type == T_NODE) {
      int idx = atoi(tokens[t_idx].lexeme);
      left = &prev_nodes[idx];
    } else {
      left = malloc(sizeof(Node));
      if (tokens[t_idx].type == T_INT) {
        Constant *c = malloc(sizeof(Constant));
        c->value = atoi(tokens[t_idx].lexeme);
        left->type = CONSTANT;
        left->data.constant = c;

      } else {
        Name *n = malloc(sizeof(Name));
        n->id = tokens[t_idx].lexeme;
        left->type = NAME;
        left->data.name = n;
      }
    }

    /* ---- fold * / chain ---- */
    while (tokens[t_idx + 1].type == T_PLUS ||
           tokens[t_idx + 1].type == T_MINUS) {

      BinaryOp *bin = malloc(sizeof(BinaryOp));
      bin->op = (tokens[t_idx + 1].type == T_PLUS) ? ADD : SUB;

      t_idx += 2;

      Node *right;
      if (tokens[t_idx].type == T_NODE) {
        int idx = atoi(tokens[t_idx].lexeme);
        right = &prev_nodes[idx];
      } else {
        right = malloc(sizeof(Node));

        if (tokens[t_idx].type == T_INT) {
          Constant *c = malloc(sizeof(Constant));
          c->value = atoi(tokens[t_idx].lexeme);
          right->type = CONSTANT;
          right->data.constant = c;

        } else {
          Name *n = malloc(sizeof(Name));
          n->id = tokens[t_idx].lexeme;
          right->type = NAME;
          right->data.name = n;
        }
      }

      bin->left = left;
      bin->right = right;

      Node *bin_node = malloc(sizeof(Node));
      bin_node->type = BINARYOP;
      bin_node->data.binary_op = bin;

      left = bin_node;
    }

    /* ---- emit node ---- */
    out_nodes[out_n_idx] = *left;

    Token t;
    t.type = T_NODE;
    t.lexeme = malloc(12);
    sprintf(t.lexeme, "%d", out_n_idx);

    out_tokens[out_t_idx++] = t;
    out_n_idx++;
    t_idx++;
  }

  out_tokens[out_t_idx].type = T_EOF;

  ParseResult result = {
    .tokens = out_tokens,
    .nodes = out_nodes,
    .node_count = out_n_idx
  };

  return result;
}

ParseResult handle_comparator(
    Token *tokens,
    Node *prev_nodes,
    int prev_node_count
) {
  int length = 0;
  while (tokens[length].type != T_EOF) length++;

  Token *out_tokens = malloc((length + 1) * sizeof(Token));
  Node  *out_nodes  = malloc(10 * sizeof(Node));

  int t_idx = 0;
  int out_t_idx = 0;
  int out_n_idx = 0;

  TokenType operand_group[3] = { T_INT, T_NAME, T_NODE };

  while (tokens[t_idx].type != T_EOF &&
         tokens[t_idx].type != T_COMMA &&
         tokens[t_idx].type != T_RPAREN &&
         tokens[t_idx].type != T_COLON &&
         tokens[t_idx].type != T_NEWLINE) {

    expect_in(tokens[t_idx].type, operand_group, 3);

    /* ---- left operand ---- */
    Node *left;

    if (tokens[t_idx].type == T_NODE) {
      int idx = atoi(tokens[t_idx].lexeme);
      left = &prev_nodes[idx];
    } else {
      left = malloc(sizeof(Node));
      if (tokens[t_idx].type == T_INT) {
        Constant *c = malloc(sizeof(Constant));
        c->value = atoi(tokens[t_idx].lexeme);
        left->type = CONSTANT;
        left->data.constant = c;

      } else {
        Name *n = malloc(sizeof(Name));
        n->id = tokens[t_idx].lexeme;
        left->type = NAME;
        left->data.name = n;
      }
    }

    /* ---- fold * / chain ---- */
    while (tokens[t_idx + 1].type == T_LT ||
           tokens[t_idx + 1].type == T_LEQ ||
           tokens[t_idx + 1].type == T_EQ ||
           tokens[t_idx + 1].type == T_GT ||
           tokens[t_idx + 1].type == T_GEQ) {

      BinaryOp *bin = malloc(sizeof(BinaryOp));
      switch (tokens[t_idx+1].type) {
        case T_EQ:
          bin->op = EQ;
          break;
        case T_LT:
          bin->op = LT;
          break;
        case T_LEQ:
          bin->op = LTE;
          break;
        case T_GT:
          bin->op = GT;
          break;
        case T_GEQ:
          bin->op = GTE;
          break;
        default:
          printf("error: non-comparison bin-op: %d\n", tokens[t_idx+1].type);
          exit(1);
          break;
      }

      t_idx += 2;

      Node *right;
      if (tokens[t_idx].type == T_NODE) {
        int idx = atoi(tokens[t_idx].lexeme);
        right = &prev_nodes[idx];
      } else {
        right = malloc(sizeof(Node));

        if (tokens[t_idx].type == T_INT) {
          Constant *c = malloc(sizeof(Constant));
          c->value = atoi(tokens[t_idx].lexeme);
          right->type = CONSTANT;
          right->data.constant = c;

        } else {
          Name *n = malloc(sizeof(Name));
          n->id = tokens[t_idx].lexeme;
          right->type = NAME;
          right->data.name = n;
        }
      }

      bin->left = left;
      bin->right = right;

      Node *bin_node = malloc(sizeof(Node));
      bin_node->type = BINARYOP;
      bin_node->data.binary_op = bin;

      left = bin_node;
    }

    /* ---- emit node ---- */
    out_nodes[out_n_idx] = *left;

    Token t;
    t.type = T_NODE;
    t.lexeme = malloc(12);
    sprintf(t.lexeme, "%d", out_n_idx);

    out_tokens[out_t_idx++] = t;
    out_n_idx++;
    t_idx++;
  }

  out_tokens[out_t_idx].type = T_EOF;

  ParseResult result = {
    .tokens = out_tokens,
    .nodes = out_nodes,
    .node_count = out_n_idx
  };

  return result;
}

Node *parse_expression(const Token *tokens, int *t_idx) {
  // one pass for each precedence level 
  ParseResult f = handle_functions(tokens, t_idx);
  ParseResult md = handle_multiply_divide(f.tokens, f.nodes, f.node_count);
  ParseResult as = handle_add_subtract(md.tokens, md.nodes, md.node_count);
  ParseResult gl = handle_comparator(as.tokens, as.nodes, as.node_count);
  return &gl.nodes[0];
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
      // null-terminate arg array
      f->args[a_idx] = NULL;
      expect(tokens[(*t_idx)++].type, T_RPAREN);
      expect(tokens[(*t_idx)++].type, T_COLON);
      expect(tokens[(*t_idx)++].type, T_NEWLINE);
      expect(tokens[(*t_idx)++].type, T_INDENT);
      f->body = parse(tokens, t_idx);
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
    } else if (tokens[*t_idx].type == T_IF) {
      (*t_idx)++;
      If *if_struct = malloc(sizeof(If));
      // parse test expr and check syntax
      if_struct->test = parse_expression(tokens, t_idx);
      expect(tokens[(*t_idx)++].type, T_COLON);
      expect(tokens[(*t_idx)++].type, T_NEWLINE);
      expect(tokens[(*t_idx)++].type, T_INDENT);
      // parse body (true block)
      if_struct->body = parse(tokens, t_idx);
      if (tokens[*t_idx].type == T_ELSE) {
        // NOTE: we expand `elif` -> `else if` in tokenization and add
        // extra INDENT + DEDENT as required (we track depth there...)
        if (tokens[++(*t_idx)].type == T_COLON) {
          // NOTE: handle colon and indent if it's a raw `else:`
          expect(tokens[++(*t_idx)].type, T_NEWLINE);
          expect(tokens[++(*t_idx)].type, T_INDENT);
          (*t_idx)++;
        }
        if_struct->orelse = parse(tokens, t_idx);
      }
      Node *if_node = malloc(sizeof(Node));
      if_node->type = IF;
      if_node->data.iff = if_struct;
      result->nodes[n_idx++] = if_node;
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
    } else if (tokens[*t_idx].type == T_DEDENT) {
      (*t_idx)++;
      break;
    } else {
      result->nodes[n_idx++] = parse_expression(tokens, t_idx);
    }
  }
  result->nodes[n_idx] = NULL;
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
  char *result = malloc(530); // TODO: grow dynamically
  char *start = result;
  if (n->type == CONSTANT) {
    result += sprintf(result, "Constant(value=%d)", n->data.constant->value);
  } else if (n->type == NAME) {
    result += sprintf(result, "Name(id='%s')", n->data.name->id);
  } else if (n->type == BINARYOP) {
    result += sprintf(
      result, 
      "BinOp(\n%sleft=%s,\n%sop=%s,\n%sright=%s\n%s)",
      space(indent+2),
      node_format(n->data.binary_op->left, indent+2), 
      space(indent+2),
      bin_op_table[n->data.binary_op->op],
      space(indent+2),
      node_format(n->data.binary_op->right, indent+2),
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
      "],\n%sbody=[%s\n",
      space(indent+2),
      space(indent+2)
    );
    for (int j=0; n->data.function_def->body->nodes[j] != NULL; j++) {
      result += sprintf(
        result,
        "%s%s",
        space(indent+4),
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
  } else if (n->type == IF) {
    result += sprintf(
      result,
      "If(\n%stest=%s,\n",
      space(indent+2),
      node_format(n->data.iff->test, indent+2)
    );
    result += sprintf(
      result,
      "%sbody=[\n",
      space(indent+2)
    );
    for (int j=0; n->data.iff->body->nodes[j] != NULL; j++) {
      result += sprintf(
        result,
        "%s%s",
        space(indent+4),
        node_format(n->data.iff->body->nodes[j], indent+4)
      );
      if (n->data.iff->body->nodes[j+1] != NULL) {
        result += sprintf(result, ",\n");
      } else {
        result += sprintf(
          result, 
          "\n%s]", 
          space(indent+2)
        );
      }
    }
    if (n->data.iff->orelse != NULL) {
      result += sprintf(
        result,
        ",\n%sorelse=[\n",
        space(indent+2)
      );
      for (int j=0; n->data.iff->orelse->nodes[j] != NULL; j++) {
        result += sprintf(
          result,
          "%s%s",
          space(indent+4),
          node_format(n->data.iff->orelse->nodes[j], indent+4)
        );
        if (n->data.iff->orelse->nodes[j+1] != NULL) {
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
    case CONSTANT: {
      PyIntObject *constant = malloc(sizeof(PyIntObject));
      constant->type = PY_INT;
      constant->value = node->data.constant->value;
      consts[*c_idx] = (PyObject *) constant;
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_CONST,%d", *c_idx);
      *b_idx += 1;
      *c_idx += 1;
      break;
    }
    case NAME:
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "LOAD_NAME,'%s'", node->data.name->id);
      *b_idx += 1;
      break;
    case BINARYOP:
      walk(node->data.binary_op->left, bytecode, b_idx, consts, c_idx);
      walk(node->data.binary_op->right, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = malloc(32); 
      sprintf(bytecode[*b_idx], "BINARY_OP,%d", node->data.binary_op->op);
      *b_idx += 1;
      break;
    case ASSIGN:
      walk(node->data.assign->value, bytecode, b_idx, consts, c_idx);
      bytecode[*b_idx] = malloc(32);
      sprintf(bytecode[*b_idx], "STORE_NAME,'%s'", node->data.assign->target->id);
      *b_idx += 1;
      break;
    case FUNCTIONDEF: {
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
    }
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
  int b_idx = 0; int c_idx = 0;
  for (int i=0; module->nodes[i] != NULL; i++) {
    walk(module->nodes[i], result->bytecode, &b_idx, result->consts, &c_idx);
  }
  result->bytecode[b_idx] = NULL;
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
  TokenArray tokens = tokenize("def foo(x):\n    if x > 0:\n        return x\n    else:\n        return 0\nprint(foo(2))");
  print_tokens(tokens.data);

  int t_idx = 0;
  Module *module = parse(tokens.data, &t_idx);
  printf("module = \n");
  module_print(module);

  printf("bytecode = \n");
  PyCodeObject *code = module_walk(module);
  for (int i=0; code->bytecode[i] != NULL; i++) {
    printf("%s\n", code->bytecode[i]);
  }
}
