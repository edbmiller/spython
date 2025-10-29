// parse epython code into an abstract syntax tree and emit bytecode
// via a post-order traversal

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parser.h"

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
      while (node->data.call_function->args[i]) {
        // load the value in this node - either name or const
        switch (node->data.call_function->args[i]->type) {
          case CONSTANT:
            PyIntObject *constant = malloc(sizeof(PyIntObject));
            constant->type = PY_INT;
            constant->value = node->data.call_function->args[i]->data.constant->value;
            consts[*c_idx] = (PyObject *) constant;
            bytecode[*b_idx] = malloc(32); 
            sprintf(bytecode[*b_idx], "LOAD_CONST,%d", *c_idx);
            *b_idx += 1;
            *c_idx += 1;
            break;
          case NAME:
            bytecode[*b_idx] = malloc(32);
            sprintf(bytecode[*b_idx], "LOAD_NAME,'%s'", node->data.call_function->args[i]->data.name->id);
            *b_idx += 1;
            break;
        }
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

        // parse args
        Node **args = malloc(5 * sizeof(Node *));
        int arg_count = 0;
        int arg_start_idx;
        i++;
        if (input[i] != ')') {
          // we have some
          arg_start_idx = i;
          while (i<len) {
            if (input[i] == ',' || input[i] == ')') {
              // arg expression is finished => create node
              args[arg_count] = parse_expression(input+arg_start_idx, i-arg_start_idx);
              // reset + continue to next
              if (input[i] == ')')
                break;
              else {
                i += 2;
                arg_start_idx = i;
                arg_count++;
              }
            }
            i++;
          }
        }
        
        // attach and finish
        call->args = args;
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
  const char *line; // assign when we see the whole line

  while (input[i] != '\0') {
    if (input[i] == '\n') {
      int len = i - line_start_offset; // our "line" ends BEFORE the \n
      line = input + line_start_offset;

      // ------ BEGIN main character crunching bit
      if (len == 6 && strncmp(line, "return", 6) == 0) {
        // RETURN NULL
        Return *ret = malloc(sizeof(Return)); // NOTE: `value` is null ptr
        Node *ret_node = malloc(sizeof(Node));
        ret_node->type = RETURN; 
        ret_node->data.ret = ret;
        module->nodes[node_idx] = ret_node;
        node_idx++;
        break; // we are in a function if we see a return => module is finished now
      } else if (len >= 8 && strncmp(line, "return ", 7) == 0) {
        // RETURN
        Return *ret = malloc(sizeof(Return));
        ret->value = parse_expression(line+7, len-7);
        Node *ret_node = malloc(sizeof(Node));
        ret_node->type = RETURN;
        ret_node->data.ret = ret;
        module->nodes[node_idx] = ret_node;
        node_idx++;
        break; // same as above
      } else if (len >= 3 && strncmp(line, "def ", 4) == 0) {

        // FUNCTIONDEF
        // find and copy the function name
        char *f_name = malloc(10); // NOTE: max function name is 10
        int k;
        for (k=4; k<len; k++) {
          if (line[k] == '(') {
            f_name[k-4] = '\0';
            break;
          }
          f_name[k-4] = line[k];
        } 

        // now parse arguments (all positional)
        // e.g. foo(a, b, c)
        k++;
        int f_arg_count = 0;
        char **f_args = malloc(5 * sizeof(char*));
        if (line[k] != ')') {
          int arg_idx = 0;
          f_args[0] = malloc(5);
          while (k < len) {
            if (line[k] == ')')
              break;
            if (line[k] == ',') {
              f_args[f_arg_count][arg_idx] = '\0';
              // reset for new arg
              f_arg_count++; 
              arg_idx = 0;
              f_args[f_arg_count] = malloc(5);
              k += 2;
            } else {
              // part of arg name 
              f_args[f_arg_count][arg_idx] = line[k]; 
              arg_idx++;
              k += 1; 
            }
          }
        }
        
        /* // DEBUGGING
        int i = 0;
        while (f_args[i] != NULL) {
          printf("%d: %s\n", i, f_args[i]);
          i++;
        }
        exit(0);
        */

        // now parse its body into a module
        // ...have to find start of next line first! 
        const char *next_line = line;
        int d = 0;
        while (*next_line != '\n')
          next_line++;
          d++;
        next_line++;
        d++;

        // malloc an int which is incremented by body parse
        // so we know where the function ends to continue from
        int *sub_travelled = malloc(sizeof(int));
        *sub_travelled = 0;
        Module *f_body = parse(next_line, depth + 1, sub_travelled);

        // done! just create the node
        FunctionDef *function_def = malloc(sizeof(FunctionDef));
        function_def->name = f_name;
        function_def->args = f_args;
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
          if (line[j] == ' ' && line[j+1] == '=' && line[j+2] == ' ') {
            // parse LHS
            Name *name = malloc(sizeof(Name));
            name->id = malloc(j+1);
            memcpy(name->id, line, j);
            name->id[j] = '\0';
            
            // parse expression on RHS and put in assignment object
            Assign *assign = malloc(sizeof(Assign));
            assign->target = name;
            assign->value = parse_expression(line+j+3, len-j-3);

            // make the wrapper node and finish
            Node *assign_node = malloc(sizeof(Node)); 
            assign_node->type = ASSIGN;
            assign_node->data.assign = assign;
            module->nodes[node_idx] = assign_node;
            node_idx++; 
            break;
          }
        }
      
        // otherwise -> pure expression
        printf("DEBUG: parsing pure expression... %.*s\n", len, line);
        module->nodes[node_idx] = parse_expression(line, len);
        node_idx++;
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

  int *travel = malloc(sizeof(int));
  *travel = 0; // don't need this, just for the call
  Module *m = parse("result = print(3)\n", 0, travel);
  PyCodeObject *c = module_walk(m);
  int i = 0;
  for (; c->bytecode[i] != NULL; i++)
    printf("%s\n", c->bytecode[i]);
   
  return 0;
}
*/
