/*

 Warpy Clock for Arduino
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
 * This clock runs 10% fast for 12 hours, then 10% slow for 12 hours. Makes the
 * day fly right by!
 *
 */

#include "base.h"

// 12 hours in seconds
#define CYCLE_LENGTH (60L*60*12)
// This is a multiple of 10% for how much swing we give
#define CYCLE_MAGNITUDE (1)

void loop() {
  unsigned char cycle_direction = 0; // fast
  unsigned long cycle_position = 0;
  while(1) {
    for(unsigned char i = 0; i < IRQS_PER_SECOND + (CYCLE_MAGNITUDE * (cycle_direction?-1:1)); i++) {
      if (i == 0)
        doTick();
      else
        doSleep();
    }
    if (cycle_position++ >= CYCLE_LENGTH) {
      cycle_position = 0;
      cycle_direction = !cycle_direction;
    }
  }
} 
