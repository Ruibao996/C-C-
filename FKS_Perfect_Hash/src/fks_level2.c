#include "../inc/fks_level2.h"
#include "../inc/hash_func.h"
#include <string.h>

fks_level2 *fks_level2_init(uint32_t size, hash_parameters parameters) {
  // TODO
  if (size == 0) {
    return NULL;
  }
  fks_level2 *table = malloc(sizeof(fks_level2));
  if (table == NULL) {
    return NULL;
  }
  table->size = size;
  table->slots = calloc(size, sizeof(uint32_t));
  table->parameters = parameters;
  if (table->slots == NULL) {
    free(table);
    return NULL;
  }
  for (uint32_t i = 0; i < size; i++) {
    table->slots[i] = FKS_LEVEL2_EMPTY;
  }
  return table;
}

fks_level2 *fks_level2_build(list_node *head, uint32_t size,
                             hash_parameters parameters) {
  if (head == NULL) {
    return NULL;
  }
  // if (size == 0) {
  //   return NULL;
  // }
  fks_level2 *table = fks_level2_init(size, parameters);
  if (table == NULL) {
    return NULL;
  }
  list_node *node = head;
  while (node != NULL) {
    // int index = hash_func(node->key, parameters, size);
    if (!fks_level2_insert(table, node->key) && !fks_level2_search(table, node->key)) {
      fks_level2_destroy(table);
      return NULL;
    }
    node = node->next;
  }
  return table;
}

bool fks_level2_insert(fks_level2 *table, uint32_t key) {
  // TODO
  if (table == NULL) {
    return false;
  }
  // if (table->size == 0) {
  //   return false;
  // }
  uint32_t index = hash_func(key, table->parameters, table->size);
  if (table->slots[index] == FKS_LEVEL2_EMPTY) {
    table->slots[index] = key;
    return true;
  }
  return false;
}

bool fks_level2_search(fks_level2 *table, uint32_t key) {
  // TODO
  if (table == NULL) {
    return false;
  }
  if (table->size == 0) {
    return false;
  }
  uint32_t index = hash_func(key, table->parameters, table->size);
  return table->slots[index] == key;
}

void fks_level2_destroy(fks_level2 *table) {
  // TODO
  if (table == NULL) {
    return;
  }
  free(table->slots);
  free(table);
}
