
/*
 * Map for getting accel function from name in an "elegant" way.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "name_map.h"

#define map_size 4

static name_func_pair name_to_func_map[map_size] = {
    {"quake", quake_accel},
    {"quake_accel", quake_accel},
    {"pow", pow_accel},
    {"pow_accel", pow_accel}};

accel_func name_map_lookup(const char *name) {
  for (int i = 0; i < map_size; ++i) {
    const name_func_pair *const p = &name_to_func_map[i];
    if (strcmp(name, p->name) == 0) {
      return p->accel;
    }
  }
  return NULL;
}
