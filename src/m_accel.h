#ifndef M_ACCEL_H
#define M_ACCEL_H

typedef struct accel_settings {
  float (*accel)(const signed char, const signed char, struct accel_settings *);
  signed char overflow_lim; /* Limit of mouse speed. */
  float base;               /* Base sensitivity */
  float offset;             /* Minimum velocity for accel */
  float upper_bound;        /* Upper bound on acceleration */
  float accel_rate;         /* Acceleration multiplier */
  float power;              /* Acceleration power */
  float game_sens;          /* In-game sensitivity (to be divided out) */
  float pre_scalar_x;       /* Scale x before applying accel*/
  float pre_scalar_y;       /* Scale y */
  float post_scalar_x;      /* Scale x after applying accel */
  float post_scalar_y;      /* Scale y */
  float carry_dx;           /* dx that was truncated when conerting to char */
  float carry_dy;           /* dy that was truncated */
} accel_settings_t;

#if PRECOMP
void precomp(accel_settings_t *);
#endif

void accelerate(signed char *, signed char *, accel_settings_t *);
float quake_accel(const signed char, const signed char, accel_settings_t *);

#endif
