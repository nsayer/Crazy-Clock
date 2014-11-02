/*

 Ventinari Clock for Arduino
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
 * This code will keep a long-term average pulse rate of 1 Hz,
 * but will do so by inserting an extra tenth of a second between ticks about half
 * the time, and after doing that 10 times, will insert an extra "stutter" tick.
 *
 */

#include "base.h"

// To screw up the rhythm when we stutter, we'll pause a little
// extra after it, but that means we will stutter with a little
// more frequency
#define PAUSE_TICKS (0)
#define TICKS_TO_GATHER (IRQS_PER_SECOND - PAUSE_TICKS)

void loop() {
  unsigned char ticks_needed = TICKS_TO_GATHER;
  while(1){
    doTick(); // 1
    doSleep(); // 2
    if (q_random() % 4) {
      // Be normal. A "second" is 10 ticks long.
      for(unsigned char i = 0; i < IRQS_PER_SECOND - 2; i++)
        doSleep();
    } else {
      // This is a special "second" - it's *11* ticks long.
      // Every tenth one, we're goging to insert a "stutter tick"
      if (--ticks_needed == 0) {
        doTick();
        for(unsigned char i = 0; i < PAUSE_TICKS; i++)
          doSleep();
        ticks_needed = TICKS_TO_GATHER;
      } else {
        doSleep();
      }
      for (unsigned char i = 0; i < IRQS_PER_SECOND - 2; i++) // yes, -2, not -3.
        doSleep();
    }
  }
}
