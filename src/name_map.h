
#ifndef NAME_MAP_H
#define NAME_MAP_H

#include "m_accel.h"

#define NAME_MAX_LEN 100

typedef struct name_to_func {
  char name[NAME_MAX_LEN];
  accel_func accel;
} name_func_pair;

accel_func name_map_lookup(const char *);

#endif
