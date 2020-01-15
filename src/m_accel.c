
#include <math.h>
#include <stdlib.h>

#include "m_accel.h"

static char get_sign(const char change_in_pos) {
  /*
   * returns 1 if change_in_pos is positive. -1 otherwise. 
   */
  return change_in_pos > 0 ? 1 : -1;
}

char pow_accel(const char change_in_pos, accel_settings_t *as) {
  /*
   * Accelerates by computing change_in_pos to some power.
   * Even powers remove the sign, so we take abs and multiply by the
   * sign after. This makes the sign remain unchanged 
   */
  const char sign = get_sign(change_in_pos);
  return sign * (pow(abs(change_in_pos), as->power));
}

char quake_accel(const char change_in_pos, accel_settings_t *as) { 
  // if vel - offset is negative, set start to zero. 
  const char dist_from_c = abs(change_in_pos) - as->offset;
  
  // start is a float as (As)^P can be slightly better approximated by 
  // rounding than by truncating
  const float start = fmax(dist_from_c, 0);

  // if unbounded accel is above upper_bound, clip it. 
  const char unbounded = round(pow(as->accel_rate * start, as->power - 1));
  const char acceleration = fmin(unbounded, as->upper_bound);

  return change_in_pos * (as->base + acceleration);
}
