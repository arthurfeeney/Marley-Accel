#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "m_accel.h"

void accelerate(signed char *dx, signed char *dy, accel_settings_t *as) {
  /*
   * Apply mouse acceleration to dx and dy with user specified settings.
   * dx and dy are updated in-place.
   * Because values get trimmed when converted to char, we use carry_d* so values
   * cut off can be added to the next call.
   * This allows for greater precision overall.
   */
  const float accelerated_sens = as->accel(*dx, *dy, as);
  const float fdx = *dx * accelerated_sens;
  const float fdy = *dy * accelerated_sens;
  // Use carry_d* because of the truncation when converting to char
  const signed char trim_dx = (signed char) (fdx + as->carry_dx);
  const signed char trim_dy = (signed char) (fdy + as->carry_dy);
  // Update deltas with their trimmed values
  *dx = trim_dx;
  *dy = trim_dy;
  // update carry values so that they can be used next iteration
  as->carry_dx = fdx - trim_dx;
  as->carry_dy = fdy - trim_dy;
}

float quake_accel(const signed char dx, const signed char dy,
                  accel_settings_t *as) {
  /*
   * Computes mouse accel with quake-like settings.
   */
  // apply limit to mouse speed
  const float clip_dx = fmin(dx, as->overflow_lim);
  const float clip_dy = fmin(dy, as->overflow_lim);
  // find velocity of the mouse
  const float vel = sqrt((float) (clip_dx*clip_dx + clip_dy*clip_dy));
  // clip change to lower bound of 0.
  const float change = fmax(vel - as->offset, 0.0);
  const float unbounded = as->game_sens + pow((as->accel_rate * change),
                                              as->power-1);
  // clip accel_sens to upper bound.
  const float bounded = fmin(unbounded, as->upper_bound);
  // account for in-game multiplier.
  const float accel_sens = bounded / as->game_sens;
  return as->base + accel_sens;
}

