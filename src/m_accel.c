#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "m_accel.h"

static float clipped_vel(float, float, float);
static float clip_delta(float, signed char);
static float limit_delta(float);

#if defined(PRECOMP) && PRECOMP + 0
static float precomp_accel_sens[UCHAR_MAX][UCHAR_MAX];
static float lookup(const int dx, const int dy) {
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
  const float pre_dx = *dx * as->pre_scalar_x;
  const float pre_dy = *dx * as->pre_scalar_y;
  // apply acceleration
#if defined(PRECOMP) && PRECOMP + 0
  const float accelerated_sens = lookup(pre_dx, pre_dy);
#else
  const float accelerated_sens = as->accel(pre_dx, pre_dy, as);
#endif
  const float fdx = *dx * accelerated_sens;
  const float fdy = *dy * accelerated_sens;
  // Apply post scalars.
  const float post_dx = fdx * as->post_scalar_x;
  const float post_dy = fdy * as->post_scalar_y;
  // Add carry from previous iteration
  const float accum_dx = limit_delta(post_dx + as->carry_dx);
  const float accum_dy = limit_delta(post_dy + as->carry_dy);
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
 * Implements quake-like accel.
 * The equation takes this form:
 *   - sens = (B + (A * (v - o)) ^ (p - 1)) / g
 */
float quake_accel(const float dx, const float dy, accel_settings_t *as) {
  // apply limit to mouse deltas if set.
  const float clip_dx = clip_delta(dx, as->overflow_lim);
  const float clip_dy = clip_delta(dy, as->overflow_lim);
  const float change = clipped_vel(clip_dx, clip_dy, as->offset);
  const float unbounded =
      as->base + pow((as->accel_rate * change), as->power - 1);
  // clip accel_sens to upper bound.
  const float bounded = fmin(unbounded, as->upper_bound);
  // account for in-game multiplier.
  const float accel_sens = bounded / as->game_sens;
  return accel_sens;
}

float pow_accel(const float dx, const float dy, accel_settings_t *as) {
  const float change = clipped_vel(dx, dy, as->offset);
  return pow((as->accel_rate * change), as->power - 1);
}

/**
 * compute velocity given deltas. Apply an offset.
 */
static float clipped_vel(float dx, float dy, float offset) {
  const float vel = sqrt(dx * dx + dy * dy);
  return fmax(vel - offset, 0.0);
}

/**
 * apply a limit to delta by clipping it.
 * This is disabled when lim is 0.
 */
static float clip_delta(float delta, signed char lim) {
  if (lim > 0) {
    return fmin(delta, lim);
  }
  return delta;
}

/**
 * used to prevent overflow when converting to signed char.
 */
float limit_delta(float delta) {
  const int sign = delta > 0 ? 1 : -1;
  if (sign < 0) {
    return fmax(SCHAR_MIN, delta);
  }
  return fmin(SCHAR_MAX, delta);
}
