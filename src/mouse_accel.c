#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "mouse_accel.h"

static inline scalar_t clipped_vel(scalar_t, scalar_t, scalar_t)
    __attribute__((const));
static inline scalar_t clip_delta(scalar_t, signed char) __attribute__((const));
static inline scalar_t limit_delta(scalar_t) __attribute((const));

#if defined(PRECOMP) && PRECOMP + 0
static scalar_t precomp_accel_sens[UCHAR_MAX][UCHAR_MAX];
static scalar_t lookup(const int dx, const int dy) {
  // shift dx and dy over by SCHAR_MIN.
  const int dx_idx = dx + -SCHAR_MIN;
  const int dy_idx = dy + -SCHAR_MIN;
  return precomp_accel_sens[dx_idx][dy_idx];
}

/**
 * Precompute accel sens for all possible combinations of dx and dy.
 * There are only 256^2 possible combinations, so this is feasible.
 */
void precomp(accel_settings_t *as) {
  printf("Performing precomp");
  for (int dx = SCHAR_MIN; dx < SCHAR_MAX; ++dx) {
    for (int dy = SCHAR_MIN; dy < SCHAR_MAX; ++dy) {
      const int dx_idx = dx + -SCHAR_MIN;
      const int dy_idx = dy + -SCHAR_MIN;
      precomp_accel_sens[dx_idx][dy_idx] = as->accel(dx, dy, as);
    }
  }
}
#endif

/**
 * Apply mouse acceleration to dx and dy with user specified settings.
 * Because values get trimmed when converted to char, we use carry_d* so
 * values cut off can be added to the next call. This allows for greater
 * precision
 * dx and dy are updated in-place.
 */
void accelerate(delta_t *dx, delta_t *dy, accel_settings_t *as) {
  const scalar_t pre_dx = *dx * as->pre_scalar_x;
  const scalar_t pre_dy = *dx * as->pre_scalar_y;
  // apply acceleration
#if defined(PRECOMP) && PRECOMP + 0
  const scalar_t accelerated_sens = lookup(pre_dx, pre_dy);
#else
  const scalar_t accelerated_sens = as->accel(pre_dx, pre_dy, as);
#endif
  const scalar_t fdx = *dx * accelerated_sens;
  const scalar_t fdy = *dy * accelerated_sens;
  // Apply post scalars.
  const scalar_t post_dx = fdx * as->post_scalar_x;
  const scalar_t post_dy = fdy * as->post_scalar_y;
  // Add carry from previous iteration
  const scalar_t accum_dx = limit_delta(post_dx + as->carry_dx);
  const scalar_t accum_dy = limit_delta(post_dy + as->carry_dy);
  // truncate before conversion to delta_t prevents small jiggles
  const delta_t trim_dx = (delta_t)truncf(accum_dx);
  const delta_t trim_dy = (delta_t)truncf(accum_dy);
  // Update deltas with their trimmed values
  *dx = trim_dx;
  *dy = trim_dy;
  // update carry values so that they can be used next iteration
  as->carry_dx = accum_dx - trim_dx;
  as->carry_dy = accum_dy - trim_dy;
}

/**
 * Implements quake-like accel. The equation takes this form:
 *   - accelerated_sens = (B + (A * (v - o)) ^ (p - 1)) / g
 */
scalar_t quake_accel(const scalar_t dx, const scalar_t dy,
                     accel_settings_t *as) {
  // apply limit to mouse deltas if set.
  const scalar_t clip_dx = clip_delta(dx, as->overflow_lim);
  const scalar_t clip_dy = clip_delta(dy, as->overflow_lim);
  const scalar_t change = clipped_vel(clip_dx, clip_dy, as->offset);
  const scalar_t unbounded =
      as->base + pow((as->accel_rate * change), as->power - 1);
  // clip accel_sens to upper bound.
  const scalar_t bounded = fmin(unbounded, as->upper_bound);
  // account for in-game multiplier.
  const scalar_t accel_sens = bounded / as->game_sens;
  return accel_sens;
}

scalar_t pow_accel(const scalar_t dx, const scalar_t dy, accel_settings_t *as) {
  const scalar_t change = clipped_vel(dx, dy, as->offset);
  return pow((as->accel_rate * change), as->power - 1);
}

/**
 * compute offset, clipped velocity given deltas.
 */
static inline scalar_t clipped_vel(scalar_t dx, scalar_t dy, scalar_t offset) {
  const scalar_t vel = sqrt(dx * dx + dy * dy);
  return fmax(vel - offset, 0.0);
}

/**
 * apply a limit to delta by clipping it.
 * If lim is 0, delta is unchanged
 */
static inline scalar_t clip_delta(scalar_t delta, signed char lim) {
  if (lim > 0) {
    return fmin(delta, lim);
  }
  return delta;
}

/**
 * used to prevent overflow when converting to signed char.
 * If delta is negative, this clips delta to the min value.
 * Otherwise, it is clipped to the max value.
 */
static inline scalar_t limit_delta(scalar_t delta) {
  if (delta < 0) {
    return fmax(SCHAR_MIN, delta);
  }
  return fmin(SCHAR_MAX, delta);
}
