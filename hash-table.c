#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "hash-table.h"

// djb2 hash
int hash(const char *str) {
  int hash = 5381;
  int c;
  
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash;
}

void node_init(Node *node, char *key, int *value) {
  node->key = key;
  node->value = value;
  node->next = NULL;
}

void hashtable_init(HashTable *htable) {
  htable->size = 8; // fixed
  htable->itemCount = 0;
  // initialise pre-head nodes
  int i;
  for (i = 0; i < htable->size; i++) {
    Node *preHead = malloc(sizeof(Node));
    node_init(preHead, NULL, NULL);
    htable->data[i] = preHead;
  }
}

void hashtable_insert(HashTable *htable, const char *key, int *value) {

  // figure out bucket
  int index = hash(key) % htable->size;
  // printf("DEBUG: insert - got index = %d\n", index);
  
  // find tail of that list
  Node *tail = htable->data[index];  
  while (tail->next != NULL) 
    tail = tail->next;

  // malloc a new node and attach
  Node *entry = malloc(sizeof(Node));
  char *keyCopy = strdup(key);
  node_init(entry, keyCopy, value);

  tail->next = entry;
}

int* hashtable_get(HashTable *htable, const char *key) {
  int index = hash(key) % htable->size;
  Node *current = htable->data[index]->next;
  while (current != NULL) {
    if (strcmp(current->key, key) == 0)
      return current->value;
    current = current->next; 
  }
  return NULL;
}

void hashtable_print(HashTable *htable) {
  // print all keys and values
  int idx;
  Node *current;

  printf("{");
  for (idx = 0; idx < htable->size; idx++) {
    // iterate over bucket printing nodes we find
    current = htable->data[idx]->next;
    while (current != NULL) {
      printf("'%s':'%d',", current->key, *current->value);
      current = current->next;
    }
  }
  printf("}\n");
}

/*
int main() {

  // test hashtable!
  HashTable htable;
  hashtable_init(&htable);
  
  // malloc some ints and insert them
  int *p = (int *) malloc(sizeof(int));
  *p = 12;
  int *q = (int *) malloc(sizeof(int));
  *q = 20;
  int *r = (int *) malloc(sizeof(int));
  *r = 33;

  hashtable_insert(&htable, "cake", p);
  hashtable_insert(&htable, "poo", q);
  hashtable_insert(&htable, "aaaa", r);

  printf("table: \n");
  hashtable_print(&htable);

  int *value = hashtable_get(&htable, "poo");
  if (value != NULL)
    printf("did lookup! %d\n", *value);
  else
    printf("lookup failed :(\n");

  return 0;
}
*/
