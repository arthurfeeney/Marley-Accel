
#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "m_accel.h"

void find_overflow_lim(accel_settings_t *as) {
  /*
   * This is unlikely to occur, but
   * If the user suddenly moves their mouse quickly, it is possible
   * that the value will overflow and the mouse will move in the wrong 
   * direction. This finds the maximum shift that will result in this 
   * overflow. This only needs to be run once during the driver setup.
   */
  for(char change_in_pos = 0; change_in_pos <= SCHAR_MAX; ++change_in_pos) {
    char accelerated_change = as->accel(change_in_pos, as);
    // if accelerated change overflowed, then the prior change in position
    // is the largest that won't overflow. 
    if(accelerated_change < 0) {
      as->overflow_lim = change_in_pos - 1;
      return;
    }
    change_in_pos++;
  }
  // it never overflowed.
  as->overflow_lim = SCHAR_MAX;
}

static char get_sign(const char change_in_pos) {
  /*
   * returns 1 if change_in_pos is positive. -1 otherwise. 
   */
  return change_in_pos > 0 ? 1 : -1;
}

static char handle_overflow(const char change_in_pos) {
  char sign = get_sign(change_in_pos);
  if(sign == -1) {
    return SCHAR_MIN;
  }
  else {
    return SCHAR_MAX;
  }
}

char accel(const char change_in_pos, accel_settings_t *as) {
  /*
   * Function to compute accel of change in pos. 
   * Checks that change_in_pos  is small enough that it won't result
   * in an overflow before applying the user-specified accel function.
   */
  if(abs(change_in_pos) > as->overflow_lim) {
    return handle_overflow(change_in_pos);
  }
  else {
    return as->accel(change_in_pos, as);
  }

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
