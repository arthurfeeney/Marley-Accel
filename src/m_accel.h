#ifndef M_ACCEL_H
#define M_ACCEL_H

typedef struct accel_settings {
  char (*accel)(char, struct accel_settings *);
  char base;         /* Base sensitivity */
  char offset;       /* Minimum velocity for accel */
  char upper_bound;  /* Upper bound on acceleration */
  float accel_rate;  /* Acceleration multiplier */
  float power;       /* Acceleration power */
  char overflow_lim; /* Fastest the mouse can move without accel overflow */
} accel_settings_t;

void find_overflow_lim(accel_settings_t *);

char accel(char, accel_settings_t *);
char pow_accel(char, accel_settings_t *);
char quake_accel(char, accel_settings_t *);

#endif
