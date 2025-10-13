// parse epython code into an abstract syntax tree and emit bytecode
// via a post-order traversal

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parser.h"

void walk(Node *node, char **output, int *offset) {
  // post-order traverse AST and emit bytecode to output
  // buffer according to the current offset
  // first: allocate string
  switch (node->type) {
    case CONSTANT:
      printf("%d: LOAD_CONST,%d\n", *offset, node->data.constant->value);
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "LOAD_CONST,%d", node->data.constant->value);
      *offset += 1; 
      break; 
    case NAME:
      printf("%d: LOAD_NAME,'%s'\n", *offset, node->data.name->id);
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "LOAD_NAME,'%s'", node->data.name->id);
      *offset += 1;
      break;
    case BINARYADD:
      walk(node->data.binary_add->left, output, offset);
      walk(node->data.binary_add->right, output, offset);
      printf("%d: ADD\n", *offset);
      output[*offset] = malloc(32); 
      output[*offset] = "ADD";
      *offset += 1;
      break;
    case ASSIGN:
      // walk expression node
      walk(node->data.assign->value, output, offset);
      printf("%d: STORE_NAME,'%s'\n", *offset, node->data.assign->target->id);
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "STORE_NAME,'%s'", node->data.assign->target->id);
      *offset += 1;
      break;
    case FUNCTIONDEF:
      // TODO: walk the arguments and LOAD_NAMEs into locals
      int function_start_bytecode_offset = *offset; // copy onto stack
      // printf("walking function...\n");
      walk(node->data.function_def->ret, output, offset);
      // after walking code logic, we emit MAKE_FUNCTION with the bytecode
      // offset to put the func object on the stack, then STORE_NAME to
      // save it to a variable. woo!
      output[*offset] = malloc(32);
      sprintf(output[*offset], "MAKE_FUNCTION,%d", function_start_bytecode_offset);
      printf("%d: MAKE_FUNCTION,%d\n", *offset, function_start_bytecode_offset);
      *offset += 1; 
      output[*offset] = malloc(32);
      printf("%d: STORE_NAME,'%s'\n", *offset, node->data.function_def->name);
      sprintf(output[*offset], "STORE_NAME,'%s'", node->data.function_def->name);
      *offset += 1;
      break;
    case RETURN:
      walk(node->data.ret->value, output, offset);
      printf("%d: RETURN\n", *offset);
      output[*offset] = malloc(32); 
      output[*offset] = "RETURN";
      *offset += 1;
      break;
  }
}

void module_print(Module *module) {
  // debugging - print node types
  printf("Module:\n");
  for (int i = 0; module->nodes[i] != NULL; i++) {
    printf(" - node: ");
    switch (module->nodes[i]->type) {
      case ASSIGN:
        printf("ASSIGN\n");
        break;
      case RETURN:
        printf("RETURN\n");
        break;
    }
  }
} 

void module_walk(Module *module, char **output, int *offset) {
  // just walk for each node
  for (int i = 0; module->nodes[i] != NULL; i++) {
    // printf("DEBUG: walking node %d\n", i);
    walk(module->nodes[i], output, offset); 
  }
}

int convert_to_int(const char *s, size_t len) {
  // multiply digits by 10*place
  int factor = pow(10, len);
  int total = 0;
  for (int i=0; i<len; i++) {
    factor = factor / 10;
    total += (s[i] - '0') * factor;
  }
  return total;
}

Node *parse_expression(const char *input, size_t len) {
  // take a slice like '1 + 2' of a line like 'x = 1 + 2'
  // and emit a AST node - recursive
  // first: find first operator
  for (int i=len; i>=0; i--) {
    // TODO: do other operators
    if (input[i] == '+') {
      // do LHS - note we start at input[i+2] and have length len-i-2
      Node *left_node = parse_expression(input, i-1); 

      // do RHS (this is a leaf i.e. const or variable name
      Node *right_node = malloc(sizeof(Node)); 
      if ((input[i+2] - '0' >= 0) && (input[i+2] - '0' < 10)) {
        // then first thing is a digit => this is an int literal
        Constant *c = malloc(sizeof(Constant));
        c->value = convert_to_int(input+i+2, len-i-2);
        right_node->type = CONSTANT;
        right_node->data.constant = c;
      } else {
        // it's a variable name
        Name *n = malloc(sizeof(Name));
        n->id = malloc(len-i-2);
        memcpy(n->id, input+i+2, len-i-2);
        right_node->type = NAME;
        right_node->data.name = n;
      }

  
      // make the output node - emit BinaryAdd
      BinaryAdd *binary_add = malloc(sizeof(BinaryAdd));
      binary_add->left = left_node; 
      binary_add->right = right_node;
      Node *result_node = malloc(sizeof(Node));
      result_node->type = BINARYADD;
      result_node->data.binary_add = binary_add;

      return result_node;
    }
  }

  // if we get here there are no operators => const or variable
  Node *node = malloc(sizeof(Node)); 
  if ((*input - '0' >= 0) && (*input - '0' < 10)) {
    // then first thing is a digit => this is an int literal
    Constant *c = malloc(sizeof(Constant));
    c->value = convert_to_int(input, len);
    node->type = CONSTANT;
    node->data.constant = c;
  } else {
    // it's a variable name
    Name *n = malloc(sizeof(Name));
    n->id = malloc(len);
    memcpy(n->id, input, len);
    node->type = NAME;
    node->data.name = n;
  }
  return node;
}

Module *parse(const char *input) {
   
  Module *module = malloc(sizeof(Module));
  int idx = 0; // index into module's node array
  const char *p = input;
  const char *lineStart = input;
  int lineNumber = 1;
  
  // NOTE: assume all lines are \n-terminated :3
  while (*p) {
    if (*p == '\n') {
      size_t len = p - lineStart;
  
      // process line ------------------------
      // printf("Line %d: %.*s\n", lineNumber, (int)len, lineStart);
      
      if (len == 6 && strncmp(lineStart, "return", 6) == 0) {
        // emit RETURN node
        Node *node = malloc(sizeof(Node));
        node->type = RETURN; 
        module->nodes[idx] = node;
        idx++;
      } else {
        for (size_t i=0; i+2<len; i++) {
          // 'a = 1 + 2'
          if (lineStart[i] == ' ' && lineStart[i+1] == '=' && lineStart[i+2] == ' ') {
            // parse LHS
            Name *name = malloc(sizeof(Name));
            // copy string to name - note `i` is the size of the name
            name->id = malloc(i+1);
            memcpy(name->id, lineStart, i);
            name->id[i] = '\0';
             
            // finally make the actual node and append
            Assign *assign = malloc(sizeof(Assign));
            assign->target = name;
            assign->value = parse_expression(lineStart+i+3, len-i-3);

            Node *node = malloc(sizeof(Node)); 
            node->type = ASSIGN;
            node->data.assign = assign;
            module->nodes[idx] = node;
            idx++;  
          } 
        }
      }

      // end of processing -------------------
      lineNumber++;
      lineStart = p + 1;
    }
    p++;
  }

  return module;
}

/*
int main() {
   
  // constants
  Constant left;
  left.value = 1;
  Node left_node;
  left_node.type = CONSTANT;
  left_node.data.constant = &left;

  Constant right;
  right.value = 2;
  Node right_node;
  right_node.type = CONSTANT;
  right_node.data.constant = &right;

  // fill children of BinaryAdd node
  BinaryAdd add;
  add.left = &left_node;
  add.right = &right_node;
  Node add_node;
  add_node.type = BINARYADD;
  add_node.data.binary_add = &add;

  // node for variable name
  Name name;
  name.id = "x";
  
  // fill assignment node   
  Assign assign;
  assign.target = &name;
  assign.value = &add_node;

  Node node;
  node.type = ASSIGN;
  node.data.assign = &assign;

  // traverse example AST
  char **output = malloc(50 * sizeof(char *));
  int *offset = malloc(sizeof(int));
  *offset = 0;
  // walk(&node, output, offset);

  // DEBUG: walk a trivial function which
  // just returns an integer

  // init and label constant node
  Node *constant_node = malloc(sizeof(Node));
  Constant *constant = malloc(sizeof(Constant)); 
  constant->value = 3;
  constant_node->data.constant = constant;
  constant_node->type = CONSTANT;

  // init and label return node and link to constant node
  Node *ret_node = malloc(sizeof(Node));
  Return *ret = malloc(sizeof(Return));
  ret->value = constant_node;
  ret_node->data.ret = ret;
  ret_node->type = RETURN;

  // init and label function_def node and link to return node
  Node *function_def_node = malloc(sizeof(Node));
  FunctionDef *function_def = malloc(sizeof(FunctionDef));
  function_def->ret = ret_node;
  char *function_name = malloc(sizeof(char *));
  function_name = "foo";
  function_def->name = function_name;
  function_def_node->data.function_def = function_def;
  function_def_node->type = FUNCTIONDEF; 

  char **output = malloc(50 * sizeof(char *));
  int *offset = malloc(sizeof(int));
  *offset = 0;
  
  walk(function_def_node, output, offset);

  // Module *module = parse("x = 1 + 2\nreturn\n"); 
  // module_walk(module, output, offset);
   
  return 0;
}
*/
