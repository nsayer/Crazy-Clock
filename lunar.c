/*

 Lunar Clock for Arduino
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

// This clock needs to run 59.06 times too slow - 12 hours worth of ticking in 29.53 days.

#define WHOLE 589
#define NUMERATOR 3
#define DENOMINATOR 5

void loop() {
  static unsigned int fractional_position = 0;
  static unsigned int tick_counter = 0;

  unsigned int limit = WHOLE;
  if (fractional_position < NUMERATOR) limit++;
  if (tick_counter++ >= limit) {
    tick_counter = 0;
    fractional_position++;
    if (fractional_position >= DENOMINATOR) fractional_position = 0;
    doTick();
  } else {
    doSleep();
  }
}
