/*

 Drifting Clock for Arduino
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

/*
 * This is a common file with the code for a clock that gains or loses
 * time at a constant rate. To build one, you just set the defines
 * and include this file.
 *
 * The macros required are:
 *
 * CYCLE_COUNT - the denominator of the fractional part
 * BASE_CYCLE_LENGTH - the whole number portion
 * NUM_LONG_CYCLES - the numerator of the fractional part
 * RUN_SLOW - define this to make the clock run slow, leave it out to run fast
 */


void loop() {
  unsigned int inner_counter = 0; // this counts to either BASE_CYCLE or BASE_CYCLE+1 before we adjust tick_counter.
  unsigned int outer_counter = 0; // this counts inner cycles from 0 to CYCLE_LENGTH
  unsigned int tick_counter = 0; // This counts SI tenths-of-a-second and we tick the clock when 0. This gets adjusted by the fraction cycles.
  while(1) {
      if (++tick_counter >= IRQS_PER_SECOND) {
        tick_counter = 0;
      }
      unsigned int this_cycle_length = BASE_CYCLE_LENGTH;
      if (outer_counter < NUM_LONG_CYCLES) this_cycle_length++; // If this is a long cycle, increment
      if (++inner_counter >= this_cycle_length) {
        inner_counter = 0;
        // It's time to skip (or add) a sleep.
        if (++outer_counter >= CYCLE_COUNT) {
          outer_counter = 0;
        }
#ifdef RUN_SLOW
        // We're inserting, rather than removing sleeps.
        doSleep();
#else
        // we're going to skip a sleep. But we don't
        // want to skip an actual tick if it's time to do that.
        // So if it's time to tick then do it, but then skip the next one
        // by incrementing i an extra time. If not, then just continue the
        // for loop without the sleep that would follow.
        if (tick_counter == 0) {
          doTick();
          tick_counter++;
        }
        continue; // That is, skip the code below.
#endif
      }
      // the inner counter did not roll over. This is an ordinary systick.
      if (tick_counter == 0)
        doTick();
      else
        doSleep();
  }
}
