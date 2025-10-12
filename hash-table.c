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

void entry_init(Entry *entry, char *key, int *value) {
  entry->key = key;
  entry->value = value;
  entry->next = NULL;
}

void hashtable_init(HashTable *htable) {
  htable->size = 8; // fixed
  htable->itemCount = 0;
  // initialise pre-head nodes
  int i;
  for (i = 0; i < htable->size; i++) {
    Entry *preHead = malloc(sizeof(Entry));
    entry_init(preHead, NULL, NULL);
    htable->data[i] = preHead;
  }
}

void hashtable_insert(HashTable *htable, const char *key, int *value) {

  // figure out bucket
  int index = hash(key) % htable->size;
  
  // find tail of that list
  Entry *tail = htable->data[index];  
  while (tail->next != NULL) 
    tail = tail->next;

  // malloc a new node and attach
  Entry *entry = malloc(sizeof(Entry));
  char *keyCopy = strdup(key);
  entry_init(entry, keyCopy, value);

  tail->next = entry;
}

int* hashtable_get(HashTable *htable, const char *key) {
  int index = hash(key) % htable->size;
  Entry *current = htable->data[index]->next;
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
  Entry *current;

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
