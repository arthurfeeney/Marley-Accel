#ifndef ACCEL_H
#define ACCEL_H

typedef struct accel_settings {
  char (*accel)(char, struct accel_settings *);
  char base;        /* Base sensitivity */
  char offset;      /* Minimum velocity for accel */
  char upper_bound; /* Upper bound on acceleration */
  float accel_rate; /* Acceleration multiplier */
  float power;      /* Acceleration power */
} accel_settings_t;

char pow_accel(char, accel_settings_t *);
char quake_accel(char, accel_settings_t *);

#endif
