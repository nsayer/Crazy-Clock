/*

 Crazy Clock for Arduino
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
 * This code will keep a long-term average pulse rate of 1 Hz, but
 * will interleve periods of normal-time, treble-time and third-time ticking.
 *
 */

#include <stdlib.h>
#include "base.h"

// These are the values for the randomly constructed instruction list
#define SLOW_SPEED 1
#define NORMAL_SPEED 2
#define FAST_SPEED 3

// This *must* be odd! It's also a bit of a balancing act between allowing
// for whackiness, but not allowing the clock to drift too far.
#define LIST_LENGTH 15

void loop() {
  unsigned long seedUpdateAfter = SEED_UPDATE_INTERVAL;
  unsigned char instruction_list[LIST_LENGTH];
  unsigned char place_in_list = LIST_LENGTH; // force a reset.
  unsigned char time_per_step = 0; // This is moot - avoids an incorrect warning
  unsigned char time_in_step = 0; // This is also moot - avoids another incorrect warning
  unsigned char tick_step_placeholder = 0;
  
  while(1){
    // The intent is for the top of this loop to be hit (about) once per second
    if (--seedUpdateAfter == 0) {
      updateSeed();
      seedUpdateAfter = SEED_UPDATE_INTERVAL;
    }
    if (place_in_list >= LIST_LENGTH) {
      // We're out of instructions. Time to make some.
      // Now that our system clock speed is so slow, this whole operation takes way too long.
      // So we're going to actually do this over the course of a second. And to make it
      // stand out less, we'll make sure that the first instruction in the list is always "normal".
      // First, get the tick out of the way - it takes 1/3 of our "thinking" time away.
      doTick();
      instruction_list[0] = NORMAL_SPEED;
      for(int i = 1; i < LIST_LENGTH; i += 2) {
        // We're going to add instructions in pairs - either a double-and-half time pair or a pair of normals.
        // Adding the half and double speed in pairs - even if they're not done adjacently (as long as they *do* get done)
        // will insure the clock will keep long-term time accurately.
        switch(random() % 2) {
          case 0:
            instruction_list[i] = SLOW_SPEED;
            instruction_list[i + 1] = FAST_SPEED;
            break;
          case 1:
            instruction_list[i] = NORMAL_SPEED;
            instruction_list[i + 1] = NORMAL_SPEED;
            break;
        }
      }
      doSleep(); // Ok, now take a break;
      // Now shuffle the array - but skip the first one.
      for(int i = 1; i < LIST_LENGTH - 1; i++) {
        unsigned char swapspot = i + (random() % (LIST_LENGTH - i));
        unsigned char temp = instruction_list[i];
        instruction_list[i] = instruction_list[swapspot];
        instruction_list[swapspot] = temp;
      }
      doSleep(); // Time to take another break;
      // This must be a multiple of 3 AND be even!
      // It also should be long enough to establish a pattern
      // before changing.
      time_per_step = ((random() % 5) + 2) * 6;
      place_in_list = 0;
      time_in_step = 0;
      // Now eat the rest of this second and then proceed as usual.
      // We ticked once and slept twice, so take 3 away from IRQS_PER_SECOND.
      for(int i = 0; i < IRQS_PER_SECOND - 3; i++) doSleep();
    }
    
    // What are we doing right now?
    // Each case must consume 10 clock ticks - that is,
    // each must call either doTick() or doSleep() a total of 10 times.  
    switch(instruction_list[place_in_list]) {
      case SLOW_SPEED:
        if (tick_step_placeholder == 1) { // Try and stick the lone tick in the middle, sort of
          doTick();
        } else {
          doSleep();
        }
        for(int i = 0; i < IRQS_PER_SECOND - 1; i++)
          doSleep();
        break;
      case NORMAL_SPEED:
        doTick();
        for(int i = 0; i < IRQS_PER_SECOND - 1; i++)
          doSleep();
        break;
      case FAST_SPEED:
        // Tick 5 times over 30 "systicks"
        for(int i = 0; i < IRQS_PER_SECOND; i++) {
          if ((IRQS_PER_SECOND * tick_step_placeholder + i) % 6 == 0) {
            doTick();
          } else {
            doSleep();
          }
        }
        break;
    }
    ++tick_step_placeholder;
    tick_step_placeholder %= 3;
    if (++time_in_step >= time_per_step) {
      time_in_step = 0;
      place_in_list++;
    }
  }
}
