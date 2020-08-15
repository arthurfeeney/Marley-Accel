#ifndef MOUSE_ACCEL_H
#define MOUSE_ACCEL_H

#define SCALAR double

typedef int32_t delta_t;
typedef SCALAR scalar_t;

typedef struct accel_settings {
  scalar_t (*accel)(const scalar_t, const scalar_t, struct accel_settings *);
  delta_t overflow_lim;   /* Limit of mouse speed. */
  scalar_t base;          /* Base sensitivity */
  scalar_t offset;        /* Minimum velocity for accel */
  scalar_t upper_bound;   /* Upper bound on acceleration */
  scalar_t accel_rate;    /* Acceleration multiplier */
  scalar_t power;         /* Acceleration power */
  scalar_t game_sens;     /* In-game sensitivity (to be divided out) */
  scalar_t pre_scalar_x;  /* Scale x before applying accel*/
  scalar_t pre_scalar_y;  /* Scale y */
  scalar_t post_scalar_x; /* Scale x after applying accel */
  scalar_t post_scalar_y; /* Scale y */
  scalar_t carry_dx;      /* dx that was truncated when conerting to char */
  scalar_t carry_dy;      /* dy that was truncated */
} accel_settings_t;

typedef scalar_t (*accel_func)(const scalar_t, const scalar_t,
                               accel_settings_t *);

#if PRECOMP
void precomp(accel_settings_t *);
#endif

void accelerate(delta_t *, delta_t *, accel_settings_t *);
scalar_t quake_accel(const scalar_t, const scalar_t, accel_settings_t *);
scalar_t pow_accel(const scalar_t, const scalar_t, accel_settings_t *);

#endif
