#include "../inc/hash_chaining.h"
#include "../inc/hash_func.h"

hash_chaining *hash_chaining_init(uint32_t size) {
  // TODO
  if (size == 0) {
    return NULL;
  }
  hash_chaining *table = malloc(sizeof(hash_chaining));
  if (table == NULL) {
    return NULL;
  }
  table->size = size;
  table->slots = calloc(size, sizeof(list_node *));
  if (table->slots == NULL) {
    free(table);
    return NULL;
  }
  table->parameters = generate_hash_parameters();
  return table;
}

void hash_chaining_insert(hash_chaining *table, uint32_t key) {
  // TODO
  if (table == NULL) {
    return;
  }
  if (table->size == 0) {
    return;
  }
  uint32_t index = hash_func(key, table->parameters, table->size);
  list_node *new_node = malloc(sizeof(list_node));
  if (new_node == NULL) {
    return;
  }
  new_node->key = key;
  new_node->next = table->slots[index];
  table->slots[index] = new_node;
}

bool hash_chaining_search(hash_chaining *table, uint32_t key) {
  // TODO
  if (table == NULL) {
    return false;
  }
  if (table->size == 0) {
    return false;
  }
  uint32_t index = hash_func(key, table->parameters, table->size);
  for (list_node *node = table->slots[index]; node != NULL; node = node->next) {
    if (node->key == key) {
      return true;
    }
  }
  return false;
}

void hash_chaining_destroy(hash_chaining *table) {
  // TODO
  if (table == NULL) {
    return;
  }
  for (uint32_t i = 0; i < table->size; i++) {
    list_node *node = table->slots[i];
    while (node != NULL) {
      list_node *temp = node;
      node = node->next;
      free(temp);
    }
  }
  free(table->slots);
  free(table);
}
