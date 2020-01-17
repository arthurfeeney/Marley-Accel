#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "m_accel.h"

void accelerate(signed char *dx, signed char *dy, accel_settings_t *as) {
  /*
   * Apply mouse acceleration to dx and dy with user specified settings.
   * dx and dy are updated in-place.
   * Because values get trimmed when converted to char, we use carry_d* so
   * values cut off can be added to the next call. This allows for greater
   * precision overall.
   */
  const float pre_dx = *dx * as->pre_scalar_x;
  const float pre_dy = *dx * as->pre_scalar_y;
  // apply acceleration
  const float accelerated_sens = as->accel(pre_dx, pre_dy, as);
  const float fdx = *dx * accelerated_sens;
  const float fdy = *dy * accelerated_sens;
  // Apply post scalars.
  const float post_dx = fdx * as->post_scalar_x;
  const float post_dy = fdy * as->post_scalar_y;
  // Add carry from previous iteration
  const float accum_dx = post_dx + as->carry_dx;
  const float accum_dy = post_dy + as->carry_dy;
  // Compute trimmed values.
  // Take floor before conversion to signed value prevents small jiggles.
  const signed char trim_dx = (signed char)floor(accum_dx);
  const signed char trim_dy = (signed char)floor(accum_dy);
  // Update deltas with their trimmed values
  *dx = trim_dx;
  *dy = trim_dy;
  // update carry values so that they can be used next iteration
  as->carry_dx = accum_dx - trim_dx;
  as->carry_dy = accum_dy - trim_dy;
}

float quake_accel(const signed char dx, const signed char dy,
                  accel_settings_t *as) {
  // apply limit to mouse speed
  const float clip_dx = fmin(dx, as->overflow_lim);
  const float clip_dy = fmin(dy, as->overflow_lim);
  // find velocity of the mouse
  const float vel = sqrt((float)(clip_dx * clip_dx + clip_dy * clip_dy));
  // clip change to lower bound of 0.
  const float change = fmax(vel - as->offset, 0.0);
  const float unbounded =
      as->base + pow((as->accel_rate * change), as->power - 1);
  // clip accel_sens to upper bound.
  const float bounded = fmin(unbounded, as->upper_bound);
  // account for in-game multiplier.
  const float accel_sens = bounded / as->game_sens;
  return accel_sens;
}
