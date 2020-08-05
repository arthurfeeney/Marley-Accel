/*
 * Many parts of this application are difficult to unit test. I.e., finding a
 * mouse can't be tested very easily. So, this just uses minunit for the accel
 * functions. http://www.jera.com/techinfo/jtns/jtn002.html. This is for
 * simplicty and to avoid any requirements for running unit tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/m_accel.h"
#include "src/marley_map.h"

/* Framework implementation */

#define mu_assert(message, test)                                               \
  do {                                                                         \
    if (!(test))                                                               \
      return message;                                                          \
  } while (0)
#define mu_run_test(test)                                                      \
  do {                                                                         \
    char *message = test();                                                    \
    tests_run++;                                                               \
    if (message)                                                               \
      return message;                                                          \
  } while (0)
extern int tests_run;

#define MAX_MSG_LEN 512
char dst[MAX_MSG_LEN];

void create_msg(const char *func_name, const char *msg, const char *value) {
  dst[0] = '\0';
  strcat(dst, func_name);
  strcat(dst, ": ");
  strcat(dst, msg);
  strcat(dst, ", actually ");
  strcat(dst, value);
}

/* Unit test suite */

int tests_run = 0;

accel_settings_t basic = {.accel = quake_accel,
                          .base = 1,
                          .offset = 4,
                          .overflow_lim = 0, // disabled when set to 0
                          .upper_bound = 90,
                          .accel_rate = 1.04,
                          .power = 2,
                          .game_sens = 1,
                          .pre_scalar_x = 1.0,
                          .pre_scalar_y = 1.0,
                          .post_scalar_x = 1.0,
                          .post_scalar_y = 1.0,
                          .carry_dx = 0,
                          .carry_dy = 0};

static char *test_quake_accel_no_change() {
  /*
   * With no movement, accel sens is the base sens.
   */
  delta_t dx = 0;
  delta_t dy = 0;
  float accel_value = quake_accel(dx, dy, &basic);
  char accel_value_str[30];
  gcvt(accel_value, 30, accel_value_str);
  create_msg(__func__, "accel_value != base", accel_value_str);
  mu_assert(dst, accel_value == basic.base);
  return 0;
}

static char *test_quake_accel_small_change() {
  /*
   * With a small change, mouse velocity won't be greater than offset,
   * so the accel sens is still the base sens.
   */
  delta_t dx = 1;
  delta_t dy = -1;
  float accel_value = quake_accel(dx, dy, &basic);
  char accel_value_str[30];
  gcvt(accel_value, 20, accel_value_str);
  create_msg(__func__, "accel_value != base", accel_value_str);
  mu_assert(dst, accel_value == basic.base);
  return 0;
}

static char *test_quake_accel_large_change() {
  /*
   * With a large change, the accel sens will not be
   * the base sens.
   */
  delta_t dx = 100;
  delta_t dy = -127;
  float accel_value = quake_accel(dx, dy, &basic);
  char accel_value_str[30];
  gcvt(accel_value, 20, accel_value_str);
  create_msg(__func__, "accel_value <= base", accel_value_str);
  mu_assert(dst, accel_value > basic.base);
  return 0;
}

static char *test_accelerate_small_change() {
  /*
   * With small or no change, deltas should not be affected.
   */
  for (delta_t i = 0; i < 4; ++i) {
    delta_t dx = i;
    delta_t dy = 0;
    accelerate(&dx, &dy, &basic);
    create_msg(__func__, "dx != i or dy != 0", "nonzero");
    mu_assert(dst, dx == i && dy == 0);
  }
  return 0;
}

static char *test_accelerate_large_change() {
  /*
   * with a large change, deltas should be scaled larger.
   */
  for (delta_t i = 110; i < 125; ++i) {
    delta_t dx = i;
    delta_t dy = i - 3;
    float accel_value = basic.accel(dx, dy, &basic);
    char accel_value_str[30];
    gcvt(accel_value, 20, accel_value_str);
    create_msg(__func__, "accel_value <= base", accel_value_str);
    mu_assert(dst, accel_value > basic.base);

    accelerate(&dx, &dy, &basic);
    create_msg(__func__, "dx or dy not scaled", "bad");
    mu_assert(dst, dx > i && dy > i - 3);
  }
  return 0;
}

static char *test_marley_map_alloc() {
  const int map_size = 10;
  marley_map *map = marley_map_alloc(map_size);
  mu_assert("map size does not match alloced", map->reserved_size == map_size);
  mu_assert("map not initialized empty", map->size == 0);
  return 0;
}

static char *test_marley_map_set_string() {
  const int map_size = 10;
  marley_map *map = marley_map_alloc(map_size);
  marley_map_set(map, "key", "value");
  mu_assert("map size not incremented to 1", map->size == 1);
  marley_map_set(map, "key2", "value");
  mu_assert("map size not incremented to 2", map->size == 2);
  marley_map_set(map, "key2", "new value");
  mu_assert("map size not incremented to 2", map->size == 2);
  return 0;
}

static char *test_marley_map_lookup() {
  const int map_size = 10;
  marley_map *map = marley_map_alloc(map_size);
  marley_map_set(map, "key", "value");
  marley_map_set(map, "key2", "value2");
  void *value = marley_map_lookup(map, "key");
  char *value_str = (char *)value;
  mu_assert("value not properley retrieved", strcmp(value_str, "value") == 0);
  void *value2 = marley_map_lookup(map, "key2");
  char *value2_str = (char *)value2;
  mu_assert("value2 not properley retrieved",
            strcmp(value2_str, "value2") == 0);
  marley_map_set(map, "key2", "different");
  void *value2_diff = marley_map_lookup(map, "key2");
  char *value2_diff_str = (char *)value2_diff;
  mu_assert("value2_diff not properley retrieved",
            strcmp(value2_diff_str, "different") == 0);

  return 0;
}

static char *all_tests() {
  mu_run_test(test_quake_accel_no_change);    // 1
  mu_run_test(test_quake_accel_small_change); // 2
  mu_run_test(test_quake_accel_large_change); // 3
  mu_run_test(test_accelerate_small_change);  // 4
  mu_run_test(test_accelerate_large_change);  // 5
  mu_run_test(test_marley_map_alloc);         // 6
  mu_run_test(test_marley_map_set_string);    // 7
  mu_run_test(test_marley_map_lookup);        // 8
  return 0;
}

int main(int argc, char **argv) {
  printf("Starting Marley Accel tests\n");
  char *result = all_tests();
  if (result == 0) {
    printf("All tests passed\n");
  } else {
    printf("Test #%d failed\n", tests_run);
    printf(" * %s\n", result);
  }
  printf("Tests run: %d\n", tests_run);
  return result != 0;
}
