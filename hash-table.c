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

void entry_init(Entry *entry, char *key, PyObject *object) {
  entry->key = key;
  entry->object = object;
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

void hashtable_insert(HashTable *htable, const char *key, PyObject *object) {

  // figure out bucket
  int index = hash(key) % htable->size;
  
  // find tail of that list
  Entry *tail = htable->data[index];  
  while (tail->next != NULL) 
    tail = tail->next;

  // malloc a new node and attach
  Entry *entry = malloc(sizeof(Entry));
  char *keyCopy = strdup(key);
  entry_init(entry, keyCopy, object);

  tail->next = entry;
}

PyObject *hashtable_get(HashTable *htable, const char *key) {
  int index = hash(key) % htable->size;
  Entry *current = htable->data[index]->next;
  while (current != NULL) {
    if (strcmp(current->key, key) == 0)
      return current->object;
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
      printf("'%s':type='%d',", current->key, current->object->type);
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
  PyIntObject *p = malloc(sizeof(PyIntObject));
  p->type = PY_INT;
  p->value = 12;

  PyIntObject *q = malloc(sizeof(PyIntObject));
  q->type = PY_INT;
  q->value = 14;

  PyFuncObject *f = malloc(sizeof(PyFuncObject));
  f->type = PY_FUNC;
  f->bytecode_offset = 10;

  hashtable_insert(&htable, "cake", (PyObject *) p);
  hashtable_insert(&htable, "poo", (PyObject *) q);
  hashtable_insert(&htable, "aaaa", (PyObject *) f);

  printf("table: \n");
  hashtable_print(&htable);

  PyObject *obj = hashtable_get(&htable, "aaaa");
  if (obj != NULL) {
    switch (obj->type) {
      case PY_INT:
        printf("got int: %d\n", ((PyIntObject *) obj)->value);
        break;
      case PY_FUNC:
        printf("got func with offset: %d\n", ((PyFuncObject *) obj)->bytecode_offset); 
        break;
    }
  } else
    printf("lookup failed :(\n");

  return 0;
}
*/
