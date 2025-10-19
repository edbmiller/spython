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
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "LOAD_CONST,%d", node->data.constant->value);
      *offset += 1; 
      break; 
    case NAME:
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "LOAD_NAME,'%s'", node->data.name->id);
      *offset += 1;
      break;
    case BINARYADD:
      walk(node->data.binary_add->left, output, offset);
      walk(node->data.binary_add->right, output, offset);
      output[*offset] = "ADD";
      *offset += 1;
      break;
    case ASSIGN:
      walk(node->data.assign->value, output, offset);
      output[*offset] = malloc(32); 
      sprintf(output[*offset], "STORE_NAME,'%s'", node->data.assign->target->id);
      *offset += 1;
      break;
    case FUNCTIONDEF:
      // TODO: walk the arguments and LOAD_NAMEs into locals
      output[*offset] = malloc(32);
      int make_function_bytecode_offset = *offset;
      *offset += 1;
      module_walk(node->data.function_def->body, output, offset);
      // now offset points at next free bytecode slot after function body
      // stack VM interprets this as "put offset+1 on stack and JUMP to arg"
      sprintf(output[make_function_bytecode_offset], "MAKE_FUNCTION,%d", *offset);
      output[*offset] = malloc(32);
      sprintf(output[*offset], "STORE_NAME,'%s'", node->data.function_def->name);
      *offset += 1;
      break;
    case RETURN:
      walk(node->data.ret->value, output, offset);
      output[*offset] = "RETURN";
      *offset += 1;
      break;
    case CALLFUNCTION:
      output[*offset] = malloc(32);
      sprintf(output[*offset], "LOAD_NAME,'%s'", node->data.call_function->func->id);
      output[++(*offset)] = "CALL_FUNCTION";
      *offset += 1;
      break;
  }
}

void module_walk(Module *module, char **output, int *offset) {
  // just walk for each node
  for (int i = 0; module->nodes[i] != NULL; i++) {
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

  // if we get here there are no operators => const or variable or callable
  Node *node = malloc(sizeof(Node)); 
  if ((*input - '0' >= 0) && (*input - '0' < 10)) {
    // then first thing is a digit => this is an int literal
    Constant *c = malloc(sizeof(Constant));
    c->value = convert_to_int(input, len);
    node->type = CONSTANT;
    node->data.constant = c;
  } else {
    // try to find a function call
    for (int i=0; i<len; i++) {
      if (input[i] == '(') {
        // make call function node
        CallFunction *call = malloc(sizeof(CallFunction)); 
        Name *func_name = malloc(sizeof(Name));
        func_name->id = malloc(i);
        memcpy(func_name->id, input, i);
        call->func = func_name;
        node->type = CALLFUNCTION; 
        node->data.call_function = call;
        return node;
      }
    }

    // otherwise it's a variable name
    Name *n = malloc(sizeof(Name));
    n->id = malloc(len);
    memcpy(n->id, input, len);
    node->type = NAME;
    node->data.name = n;
  }
  return node;
}

Module *parse(const char *input, int depth, int *travelled) {
  // parse the slice input[*offset:<end>] - we assume
  // all lines we see are indented by 4 * depth
  Module *module = malloc(sizeof(Module));
  int node_idx = 0; // idx in module node array
  int line_start_offset = depth*4; // beginning of the line being scanned + parsed
  int i = 0; // char index in the input
  const char *line_start; // assign when we see the whole line

  while (input[i] != '\0') {
    if (input[i] == '\n') {
      int len = i - line_start_offset; // our "line" ends BEFORE the \n
      line_start = input + line_start_offset;
      // printf("Parsing line: %.*s\n", len, line_start);

      // ------ BEGIN main character crunching bit
      if (len == 6 && strncmp(line_start, "return", 6) == 0) {
        // RETURN NULL
        Return *ret = malloc(sizeof(Return)); // NOTE: `value` is null ptr
        Node *ret_node = malloc(sizeof(Node));
        ret_node->type = RETURN; 
        ret_node->data.ret = ret;
        module->nodes[node_idx] = ret_node;
        node_idx++;
        break; // we are in a function if we see a return => module is finished now
      } else if (len >= 8 && strncmp(line_start, "return ", 7) == 0) {
        // RETURN
        Return *ret = malloc(sizeof(Return));
        ret->value = parse_expression(line_start+7, len-7);
        Node *ret_node = malloc(sizeof(Node));
        ret_node->type = RETURN;
        ret_node->data.ret = ret;
        module->nodes[node_idx] = ret_node;
        node_idx++;
        break; // same as above
      } else if (len >= 3 && strncmp(line_start, "def ", 4) == 0) {

        // FUNCTIONDEF
        // find and copy the function name
        char *f_name = malloc(10); // NOTE: max function name is 10
        for (int k=4; k<len; k++) {
          if (line_start[k] == '(') 
            break;
          f_name[k-4] = line_start[k];
        } 

        // now parse its body into a module
        // ...have to find start of next line first! 
        const char *next_line_start = line_start;
        int d = 0;
        while (*next_line_start != '\n')
          next_line_start++;
          d++;
        next_line_start++;
        d++;

        // malloc an int which is incremented by body parse
        // so we know where the function ends to continue from
        int *sub_travelled = malloc(sizeof(int));
        *sub_travelled = 0;
        Module *f_body = parse(next_line_start, depth + 1, sub_travelled);

        // done! just create the node
        FunctionDef *function_def = malloc(sizeof(FunctionDef));
        function_def->name = f_name;
        function_def->body = f_body;
        Node *f_def_node = malloc(sizeof(Node));
        f_def_node->type = FUNCTIONDEF;
        f_def_node->data.function_def = function_def;

        // and insert in upper module
        module->nodes[node_idx] = f_def_node;
        node_idx++; 
        
        // bump i and travelled by distance sub_travelled on 
        // body parse
        i += (d + *sub_travelled - 1);
        // printf("sub-parse done - now we have left to parse: %s", input + i + 1);
      } else {
        // ASSIGN
        for (int j=0; j+2<len; j++) {
          // look for assignment operator ' = ' 
          if (line_start[j] == ' ' && line_start[j+1] == '=' && line_start[j+2] == ' ') {
            // parse LHS
            Name *name = malloc(sizeof(Name));
            name->id = malloc(j+1);
            memcpy(name->id, line_start, j);
            name->id[j] = '\0';
            
            // parse expression on RHS and put in assignment object
            Assign *assign = malloc(sizeof(Assign));
            assign->target = name;
            assign->value = parse_expression(line_start+j+3, len-j-3);

            // make the wrapper node and finish
            Node *assign_node = malloc(sizeof(Node)); 
            assign_node->type = ASSIGN;
            assign_node->data.assign = assign;
            module->nodes[node_idx] = assign_node;
            node_idx++; 
          }
        }
      }
      // new line starts on i + 1
      line_start_offset = i + 1 + depth*4;
    }
    i++;
    (*travelled)++;
  }
  return module;
}

/*
int main() {

  char **output = malloc(50 * sizeof(char *));
  int *offset = malloc(sizeof(int));
  *offset = 0;

  int *travel = malloc(sizeof(int));
  *travel = 0; // don't need this, just for the call
  Module *m = parse("def foo():\n    x = 1 + 2\n    return x\ny = foo()\n", 0, travel);
  module_walk(m, output, offset);
   
  for (int i=0; output[i] != NULL; i++)
    printf("%d: %s\n", i, output[i]);

  return 0;
}
*/
