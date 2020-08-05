/**
 * Implementation of a mapping from string to anything.
 */

#ifndef MARLEY_MAP_H
#define MARLEY_MAP_H

typedef struct {
  char *key;
  void *value;
} key_value_pair;

key_value_pair *key_value_pair_constr(char *key, void *data);
void key_value_pair_free(key_value_pair *kv);

typedef struct {
  key_value_pair *data;
  int reserved_size;
  int size;
} marley_map;

marley_map *marley_map_alloc(int size);
void marley_map_free(marley_map *map);

void marley_map_set(marley_map *map, char *key, void *data);
void *marley_map_lookup(marley_map *map, char *key);

#endif
