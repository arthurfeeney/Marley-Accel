
#include <stdlib.h>
#include <string.h>

#include "marley_map.h"

key_value_pair *key_value_pair_constr(char *key, void *value) {
  key_value_pair *kv = malloc(sizeof(key_value_pair));
  kv->key = key;
  kv->value = value;
  return kv;
}

void key_value_pair_free(key_value_pair *kv) {
  free(kv->key);
  free(kv->value);
}

marley_map *marley_map_alloc(int size) {
  marley_map *map = malloc(sizeof(marley_map));
  map->data = malloc(sizeof(key_value_pair) * size);
  map->reserved_size = size;
  map->size = 0;
  return map;
}

void marley_map_free(marley_map *map) {
  for (int idx = 0; idx < map->size; ++idx) {
    key_value_pair *kv = &map->data[idx];
    key_value_pair_free(kv);
  }
  free(map->data);
}

void marley_map_set(marley_map *map, char *key, void *value) {
  if (map->size >= map->reserved_size) {
    return;
  }

  // check that key exists in the map
  for (int idx = 0; idx < map->size; ++idx) {
    key_value_pair *kv = &map->data[idx];
    if (strcmp(kv->key, key) == 0) {
      kv->value = value;
      return;
    }
  }

  // key doesn't exist, so add it in.
  const int insert_idx = map->size;
  key_value_pair insert_value = {.key = key, .value = value};
  map->data[insert_idx] = insert_value;
  ++map->size;
}

void *marley_map_lookup(marley_map *map, char *key) {
  for (int idx = 0; idx < map->size; ++idx) {
    const key_value_pair *kv = &map->data[idx];
    if (strcmp(kv->key, key) == 0) {
      return kv->value;
    }
  }
  return NULL;
}
