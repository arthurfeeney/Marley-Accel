
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "marley_map.h"

key_value_pair *key_value_pair_constr(char *key, void *value) {
  key_value_pair *kv = malloc(sizeof(key_value_pair));
  kv->key = malloc(sizeof(char) * strlen(key));
  kv->value = malloc(sizeof(value));
  *kv->key = *key;
  *((char *)kv->value) = *((char *)value);
  return kv;
}

void key_value_pair_write(key_value_pair *kv, char *key, void *value) {
  kv->key = malloc(sizeof(char) * strlen(key));
  kv->value = malloc(sizeof(value));
  *kv->key = *key;
  *((char *)kv->value) = *((char *)value);
}

void key_value_pair_free(key_value_pair *kv) {
  free(kv->key);
  free(kv->value);
}

marley_map *marley_map_alloc(int size) {
  if (size <= 0) {
    return NULL;
  }
  marley_map *map = malloc(sizeof(marley_map));
  map->data = malloc(sizeof(key_value_pair) * size);
  map->reserved_size = size;
  map->size = 0;
  return map;
}

void marley_map_free(marley_map *map) {
  free(map->data);
  free(map);
}

int marley_map_resize(marley_map *map, int new_reserved_size) {
  if (new_reserved_size < map->size) {
    return MARLEY_MAP_RESIZE_FAILED;
  } else if (new_reserved_size <= 0) {
    return MARLEY_MAP_RESERVED_SIZE_NOT_POSITIVE;
  }

  const int size = map->size;
  if (size == 0) {
    free(map->data);
    map->data = NULL;
    map->data = malloc(sizeof(key_value_pair) * new_reserved_size);
    map->reserved_size = new_reserved_size;
    // map->size = size;
    return 0;
  }

  key_value_pair *replace = malloc(sizeof(key_value_pair) * new_reserved_size);
  for (int idx = 0; idx < size; ++idx) {
    replace[idx] = map->data[idx];
  }
  free(map->data);
  map->data = NULL;
  map->data = malloc(sizeof(key_value_pair) * new_reserved_size);

  for (int idx = 0; idx < size; ++idx) {
    map->data[idx] = replace[idx];
  }
  map->size = size;
  map->reserved_size = new_reserved_size;
  free(replace);
  return 0;
}

/**
 * set the mapping from key to value.
 * If the map's reserved memory is full, it doubles the reserved size before
 * setting the mapping
 */
int marley_map_set(marley_map *map, char *key, void *value) {
  if (map->size == map->reserved_size) {
    int err = marley_map_resize(map, map->reserved_size * 2);
    if (err) {
      return err;
    }
  }

  // if key already exists in the map, update its value
  for (int idx = 0; idx < map->size; ++idx) {
    key_value_pair *kv = &map->data[idx];
    if (strcmp(kv->key, key) == 0) {
      kv->value = value;
      return 0;
    }
  }

  // key doesn't exist, so set it.
  const int insert_idx = map->size;
  key_value_pair insert_value = {.key = key, .value = value};
  map->data[insert_idx] = insert_value;
  ++map->size;
  return 0;
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
