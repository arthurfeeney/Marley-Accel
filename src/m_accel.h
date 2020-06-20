#ifndef M_ACCEL_H
#define M_ACCEL_H

typedef int delta_t;

typedef struct accel_settings {
  float (*accel)(const float, const float, struct accel_settings *);
  delta_t overflow_lim; /* Limit of mouse speed. */
  float base;           /* Base sensitivity */
  float offset;         /* Minimum velocity for accel */
  float upper_bound;    /* Upper bound on acceleration */
  float accel_rate;     /* Acceleration multiplier */
  float power;          /* Acceleration power */
  float game_sens;      /* In-game sensitivity (to be divided out) */
  float pre_scalar_x;   /* Scale x before applying accel*/
  float pre_scalar_y;   /* Scale y */
  float post_scalar_x;  /* Scale x after applying accel */
  float post_scalar_y;  /* Scale y */
  float carry_dx;       /* dx that was truncated when conerting to char */
  float carry_dy;       /* dy that was truncated */
} accel_settings_t;

typedef float (*accel_func)(const float, const float, accel_settings_t *);

#if PRECOMP
void precomp(accel_settings_t *);
#endif

void accelerate(delta_t *, delta_t *, accel_settings_t *);
float quake_accel(const float, const float, accel_settings_t *);
float pow_accel(const float, const float, accel_settings_t *);

#endif
