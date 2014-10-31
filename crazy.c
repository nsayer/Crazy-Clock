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

#include <string.h>
#include "base.h"

// These are the values for the randomly constructed instruction list
#define SLOW_SPEED 1
#define NORMAL_SPEED 2
#define FAST_SPEED 3

// This *must* be even! It's also a bit of a balancing act between allowing
// for whackiness, but not allowing the clock to drift too far.
#define LIST_LENGTH 14

// Picking random numbers takes so long (at our slow clock speed) that
// we can only afford to pick one every tenth of a second. So we're going
// to keep a cache of them. For simplicity, it's a FIFO cache (they're
// random numbers, after all). Make sure that the buffer is at least enough
// to satisfy the need of the list regeneration code, which is to say
// 1.5 * LIST_LENGTH + 1. However, we only need random chars, but
// what we get from q_random() is random longs. So we only really need
// a quarter that many.

#define BUF_LEN (2 * LIST_LENGTH)

static unsigned char buf_ptr = 0;
static unsigned char random_buf[BUF_LEN];
static unsigned char instruction_list_stage[LIST_LENGTH];

static unsigned char buf_random() {
  if (buf_ptr > BUF_LEN - 4) return 1; // buffer is full
  unsigned long val = q_random();
  random_buf[buf_ptr++] = (unsigned char)(val >> 24);
  random_buf[buf_ptr++] = (unsigned char)(val >> 16);
  random_buf[buf_ptr++] = (unsigned char)(val >> 8);
  random_buf[buf_ptr++] = (unsigned char)val;
  return 0;
}

static unsigned char our_random() {
  if (buf_ptr == 0) return 0; // what else can we do?
  return random_buf[--buf_ptr];
}

static void our_sleep() {
  buf_random();
  doSleep();
}

static void our_tick() {
  //buf_random(); // no - ticking already takes 35 ms.
  doTick();
}

static void build_list(int which) {
  int start = 0, end = 0;
  switch(which) {
    case 0:
      start = 0;
      end = LIST_LENGTH / 2;
      break;
    case 1:
      start = LIST_LENGTH / 2;
      end = LIST_LENGTH;
      break;
  }
  for(int i = start; i < end; i += 2) {
    // We're going to add instructions in pairs - either a double-and-half time pair or a pair of normals.
    // Adding the half and double speed in pairs - even if they're not done adjacently (as long as they *do* get done)
    // will insure the clock will keep long-term time accurately.
    switch(our_random() % 2) {
      case 0:
        instruction_list_stage[i] = SLOW_SPEED;
        instruction_list_stage[i + 1] = FAST_SPEED;
        break;
      case 1:
        instruction_list_stage[i] = NORMAL_SPEED;
        instruction_list_stage[i + 1] = NORMAL_SPEED;
        break;
    }
  }
}

static void shuffle_list(int which) {
  int start = 0, end = 0;
  switch(which) {
    case 0:
      start = LIST_LENGTH - 1;
      end = LIST_LENGTH / 2;
      break;
    case 1:
      start = LIST_LENGTH / 2;
      end = 0;
      break;
  }
  // Now shuffle the array - classic Knuth shuffle
  for(int i = start; i != end; i--) {
    unsigned char swapspot = our_random() % (i + 1);
    unsigned char temp = instruction_list_stage[i];
    instruction_list_stage[i] = instruction_list_stage[swapspot];
    instruction_list_stage[swapspot] = temp;
  }
}

void loop() {
  unsigned char instruction_list[LIST_LENGTH];
  unsigned char place_in_list = LIST_LENGTH; // force a reset.
  unsigned char time_per_step = 0; // This is moot - avoids an incorrect warning
  unsigned char time_in_step = 0; // This is also moot - avoids another incorrect warning
  unsigned char tick_step_placeholder = 0;
  unsigned char rebuilding_state = 0; // not rebuilding

  // Fill the random number cache
  while (!buf_random()) ;
 
  // build the initial list. The clock hasn't started yet, so it doesn't matter how long this takes. 
  build_list(0);
  build_list(1);
  shuffle_list(0);
  shuffle_list(1);

  // Now we start the clock for real.
  while(1){
    if (place_in_list >= LIST_LENGTH) {
      // We're out of instructions. Grab the staged list and set the rebuilding machine in motion.
      memcpy(instruction_list, instruction_list_stage, sizeof(instruction_list));
      // This must be a multiple of 3 AND be even!
      // It also should be long enough to establish a pattern
      // before changing.
      time_per_step = ((our_random() % 5) + 2) * 6; // 12 - 36
      place_in_list = 0;
      time_in_step = 0;
      rebuilding_state = 1;
    }

    // Rebuild the staged instruction list. Do this in phases across multiple seconds so that
    // we don't blow past interrupts.
    switch(rebuilding_state) {
      case 0: // not rebuilding
              break;
      case 1: // Skip. We just copied the staging list. That alone took enough time.
              // Also, if this is the very first second, then the random buffer is
              // empty after the bootstrapping. 9 sleeps should be plenty to replenish
              // it (since we gain 4 random numbers every time).
              break;
      case 2:
          build_list(0);
          break;
      case 3:
          build_list(1);
          break;
      case 4:
          shuffle_list(0);
          break;
      case 5:
          shuffle_list(1);
          // all done.
          rebuilding_state = 0;
          break;
    }
    if (rebuilding_state != 0) rebuilding_state++;
    
    // What are we doing right now?
    // Each case must consume 10 clock ticks - that is,
    // each must call either our_tick() or our_sleep() a total of 10 times.  
    switch(instruction_list[place_in_list]) {
      case SLOW_SPEED:
        if (tick_step_placeholder == 1) { // Try and stick the lone tick in the middle, sort of
          our_tick();
        } else {
          our_sleep();
        }
        for(int i = 0; i < IRQS_PER_SECOND - 1; i++)
          our_sleep();
        break;
      case NORMAL_SPEED:
        our_tick();
        for(int i = 0; i < IRQS_PER_SECOND - 1; i++)
          our_sleep();
        break;
      case FAST_SPEED:
        // Tick 5 times over 30 "systicks"
        for(int i = 0; i < IRQS_PER_SECOND; i++) {
          if ((IRQS_PER_SECOND * tick_step_placeholder + i) % 6 == 0) {
            our_tick();
          } else {
            our_sleep();
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
