// parse epython code into an abstract syntax tree and emit bytecode
// via a post-order traversal

// TODO: implement expression parsing for the RHS of assignments
// then we can actually do the full pipeline :O :O

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// NOTE: define our nodes
typedef enum NodeType {
  CONSTANT,
  NAME,
  BINARYADD,
  ASSIGN,
  RETURN
} NodeType;

typedef struct Constant {
  int value; // TODO: pointer
} Constant;

typedef struct Name {
  char *id;
} Name;

typedef struct Assign {
  Name *target;
  struct Node *value;
} Assign;

typedef struct BinaryAdd {
  struct Node *left;
  struct Node *right;
} BinaryAdd;

typedef struct Node {
  NodeType type;
  union {
    Constant *constant;
    Name *name;
    BinaryAdd *binary_add;
    Assign *assign;
  } data;
} Node;

void walk(Node *node) {
  // post-order traverse AST and emit bytecode
  // printf("DEBUG: walk - switching on node type %d\n", node->type);
  switch (node->type) {
    case CONSTANT:
      printf("LOAD_CONST,%d\n", node->data.constant->value);
      break; 
    case NAME:
      printf("LOAD_NAME,'%s'\n", node->data.name->id);
      break;
    case BINARYADD:
      walk(node->data.binary_add->left);
      walk(node->data.binary_add->right);
      printf("ADD\n");
      break;
    case ASSIGN:
      // walk expression node
      walk(node->data.assign->value);
      printf("STORE_NAME,'%s'\n", node->data.assign->target->id);
      break;
    case RETURN:
      printf("RETURN\n");
      break;
  }
}

// a module is an array of AST nodes
typedef struct Module {
  Node *nodes[20];
} Module;

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

void module_walk(Module *module) {
  // just walk for each node
  for (int i = 0; module->nodes[i] != NULL; i++) {
    // printf("DEBUG: walking node %d\n", i);
    walk(module->nodes[i]); 
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

  // first: find first operator (+)
  for (int i=0; i<len; i++) {
    // TODO: do other operators
    if (input[i] == '+') {
      // do LHS (assume it's a const with a space before +)
      Constant *left = malloc(sizeof(Constant));
      left->value = convert_to_int(input, i-1);
      Node *left_node = malloc(sizeof(Node));
      left_node->type = CONSTANT;
      left_node->data.constant = left;

      // do RHS
      Constant *right = malloc(sizeof(Constant));
      right->value = convert_to_int(input+i+2, len-i-2);  
      Node *right_node = malloc(sizeof(Node));
      right_node->type = CONSTANT;
      right_node->data.constant = right;
  
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

int main() {
   
  /*
  // constants
  Constant left;
  left.value = 1;
  Node leftNode;
  leftNode.type = CONSTANT;
  leftNode.data.constant = &left;

  Constant right;
  right.value = 2;
  Node rightNode;
  rightNode.type = CONSTANT;
  rightNode.data.constant = &right;

  // fill children of BinaryAdd node
  BinaryAdd add;
  add.left = &leftNode;
  add.right = &rightNode;
  Node addNode;
  addNode.type = BINARYADD;
  addNode.data.binaryAdd = &add;

  // node for variable name
  Name name;
  name.id = "x";
  
  // fill assignment node   
  Assign assign;
  assign.target = &name;
  assign.value = &addNode;

  Node node;
  node.type = ASSIGN;
  node.data.assign = &assign;
  */

  // traverse example AST
  // walk(&node);

  Module *module = parse("x = 1 + 2\nreturn\n"); 
  module_walk(module);
   
  return 0;
}
