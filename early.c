/*

 Early Clock for Arduino
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
 * This clock runs 20% fast for 50 minutes, then 10% slow for 100 minutes.
 * The idea is that this is for folks who set their clock 10 minutes fast
 * to keep from being late to things, but then find themselves compensating.
 *
 * This clock will be anywhere from on-time to 10 minutes fast... but you
 * won't be able to predict it (without comparing it to a proper clock).
 *
 */

#include "base.h"

// 50 minutes in seconds - at 20% fast, that's 10 minutes error.
#define FAST_CYCLE_LENGTH (60L*50)
// This is a multiple of 10% for how much swing we give
#define FAST_CYCLE_MAGNITUDE (-2)
#define NORMAL_CYCLE_MAGNITUDE (0)
// This is twice as long as the FAST cycle because it's half the error rate
#define SLOW_CYCLE_LENGTH (FAST_CYCLE_LENGTH * 2)
#define SLOW_CYCLE_MAGNITUDE -(FAST_CYCLE_MAGNITUDE / 2)

void loop() {
  // we have a four-state machine. Between each interval of fast or slow
  // ticking, we put a short period of normal ticking so that the transition
  // isn't obvious.
  unsigned char state = 99; // force a reset
  unsigned long current_cycle_position = 99; // force a reset
  unsigned long current_cycle_length = 0; // squelch bogus warning
  char current_cycle_magnitude = 0; // squelch bogus warning
  while(1) {
    if (current_cycle_position++ >= current_cycle_length) {
      if (++state > 3) state = 0;
      current_cycle_position = 0;
      switch(state) {
        case 0:
        case 2:
          current_cycle_length = 30 + (q_random() % 30); // shift it around a lot.
          current_cycle_magnitude = NORMAL_CYCLE_MAGNITUDE;
          break;
        case 1:
          current_cycle_length = FAST_CYCLE_LENGTH;
          current_cycle_magnitude = FAST_CYCLE_MAGNITUDE;
          break;
        case 3:
          current_cycle_length = SLOW_CYCLE_LENGTH;
          current_cycle_magnitude = SLOW_CYCLE_MAGNITUDE;
          break;
      }
    }
    for(unsigned char i = 0; i < IRQS_PER_SECOND + current_cycle_magnitude; i++) {
      if (i == 0)
        doTick();
      else
        doSleep();
    }
  }
} 
