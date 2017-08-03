/*

 Slow Clock common code
 Copyright 2014 Nicholas W. Sayer
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "base.h"

/*

This is the base for the clocks that run very slowly. To define one of
these clocks, you must specify how many 10 Hz periods occur between ticks.
In general, if you want a clock that runs N times slower than normal, then
the correct value is (N - 1) * 10.

This value can be a fraction, if desired. If it is, then the fractional
value will be spread across the count in the customary manner.

If the fractional part is zero, then set the NUMERATOR to 0. The DENOMINATOR
will not be used.
*/

#define START_TICKS (30 * 5)

void loop() {
#ifndef UNIT_TEST
  // When the battery is installed, do a whole bunch of really fast
  // ticking as a proof of life.
  for(int i = 0; i < START_TICKS; i++) {
    doTick();
    doSleep();
  }
#endif

  while(1) {
#if NUMERATOR != 0
    static unsigned int fractional_position = 0;
#endif
    static unsigned int tick_counter = 0;

    unsigned int limit = WHOLE;
#if NUMERATOR != 0
    if (fractional_position < NUMERATOR) limit++;
#endif
    if (tick_counter++ >= limit) {
      tick_counter = 0;
#if NUMERATOR != 0
      fractional_position++;
      if (fractional_position >= DENOMINATOR) fractional_position = 0;
#endif
      doTick();
    } else {
      doSleep();
    }
  }
}
