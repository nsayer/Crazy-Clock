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
 * This is intended to run on an ATTiny85. Connect a 16.384 MHz crystal and fuse it
 * for divide-by-8 clocking, no watchdog or brown-out detector.
 *
 * Connect PB0 and PB1 to the coil pins of a Lavet stepper coil of a clock movement
 * (with a series resistor and flyback diode to ground on each pin) and power it 
 * from a 3.3 volt boost converter.
 *
 * It will keep a long-term average pulse rate of 1 Hz (alternating coil pins), but
 * will interleve periods of double-time and half-time ticking.
 *
 */
 
#include <Arduino.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Turn this on to use the unused output as a debug output
// #define DEBUG

// Update the PRNG seed daily
#define SEED_UPDATE_INTERVAL (86400)

// These are the values for the randomly constructed instruction list
#define SLOW_SPEED 1
#define NORMAL_SPEED 2
#define FAST_SPEED 3

// This *must* be even! It's also a bit of a balancing act between allowing
// for whackiness, but not allowing the clock to drift too far.
#define LIST_LENGTH 14

// We're going to set up a timer interrupt for every 100 msec, so what's 1/.1?
// Note that while we're actually doing stuff, we *must* insure that we never
// work through an interrupt. This is because we're not *counting* these
// interrupts, we're just waiting for each one in turn.
#define IRQS_PER_SECOND (10)

// Our design choices may wind up with a system clock of either 500 kHz
// or 512 kHz. If it's 500 kHz, then we have to do some juggling to wind up
// with the proper IRQS_PER_SECOND value of 10. To set that up, uncomment
// this:
// #define TEN_BASED_CLOCK

#ifdef TEN_BASED_CLOCK
// 50,000 divided by 1024 is 48 53/64, which is 49*53 + 48*(64-53)
#define CLOCK_CYCLES (64)
// Don't forget to decrement the OCR0A value - it's 0 based and inclusive
#define CLOCK_BASIC_CYCLE (48 - 1)
// a "long" cycle is CLOCK_BASIC_CYCLE + 1
#define CLOCK_NUM_LONG_CYCLES (53)
#endif

// clock solenoid pins
#define P0 0
#define P1 1
#define P_UNUSED 2

// How long is each tick? In this case, we're going to busy-wait on the timer.
#define TICK_LENGTH (35)

// This will alternate the ticks
#define TICK_PIN (lastTick == P0?P1:P0)

static unsigned char lastTick;

// This delay loop is magical because we know the timer is ticking at 500 Hz.
// So we just wait until it counts N/2 times and that will be an N msec delay.
static void delay_ms(unsigned char msec) {
   unsigned char start_time = TCNT0;
   while(TCNT0 - start_time < msec / 2) ; // sit-n-spin
}

#ifdef TEN_BASED_CLOCK
  static unsigned char cycle_pos = 0xff; // force a reset
#endif

static void doSleep() {
#ifdef TEN_BASED_CLOCK
  if (cycle_pos == CLOCK_NUM_LONG_CYCLES)
    OCR0A = CLOCK_BASIC_CYCLE;
  if (cycle_pos++ >= CLOCK_CYCLES) {
    OCR0A = CLOCK_BASIC_CYCLE + 1;
    cycle_pos = 0;
  }
#endif
  sleep_mode();
}

// Each call to doTick() will "eat" a single one of our interrupt "ticks"
static void doTick() {
  digitalWrite(TICK_PIN, HIGH);
  delay_ms(TICK_LENGTH);
  digitalWrite(TICK_PIN, LOW);
  lastTick = TICK_PIN;
  doSleep(); // eat the rest of this tick
}

static void updateSeed() {
  unsigned long seed = random();
  EEPROM.write(0, (char)seed);
  EEPROM.write(1, (char)(seed >> 8));
  EEPROM.write(2, (char)(seed >> 16));
  EEPROM.write(3, (char)(seed >> 24));
}

ISR(TIMER0_COMPA_vect) {
  // do nothing - just wake up
}

void setup() {
  // change this so that we wind up with a 512 kHz or a 500 kHz CPU clock.
  // And if it's 500 kHz, be sure to uncomment TEN_BASED_CLOCK above.
  clock_prescale_set(clock_div_8);
  power_adc_disable();
  power_usi_disable();
  power_timer1_disable();
  TCCR0A = _BV(WGM01); // mode 2 - CTC
  TCCR0B = _BV(CS02) | _BV(CS00); // prescale = 1024
#ifndef TEN_BASED_CLOCK
  // count freq = 512 kHz / 1024 = 500 Hz
  OCR0A = 49; // 10 Hz - don't forget to subtract 1 - the counter is 0-49.
#endif
  TIMSK = _BV(OCIE0A); // OCR0A interrupt only.
  ACSR = _BV(ACD); // Turn off analog comparator - but was it ever on anyway?
  
  set_sleep_mode(SLEEP_MODE_IDLE);

#ifdef DEBUG
  pinMode(P_UNUSED, OUTPUT);
  digitalWrite(P_UNUSED, LOW);
#else
  pinMode(P_UNUSED, INPUT_PULLUP);
#endif
  pinMode(P0, OUTPUT);
  pinMode(P1, OUTPUT);
  digitalWrite(P0, LOW);
  digitalWrite(P1, LOW);
  
  lastTick = P0;
    
  // Try and perturb the PRNG as best as we can
  unsigned long seed = EEPROM.read(0);
  seed |= ((unsigned long)EEPROM.read(1))<<8;
  seed |= ((unsigned long)EEPROM.read(2))<<16;
  seed |= ((unsigned long)EEPROM.read(3))<<24;
  randomSeed(seed);
  updateSeed();
}

void loop() {
  unsigned long seedUpdateAfter = SEED_UPDATE_INTERVAL;
  unsigned char instruction_list[LIST_LENGTH];
  unsigned char place_in_list = LIST_LENGTH; // force a reset.
  unsigned char time_per_step = 0; // This is moot - avoids an incorrect warning
  unsigned char time_in_step = 0; // This is also moot - avoids another incorrect warning
  unsigned char tick_step_placeholder = 0;
  
  while(1){
    // The intent is for the top of this loop to be hit once per second
    if (--seedUpdateAfter == 0) {
      updateSeed();
      seedUpdateAfter = SEED_UPDATE_INTERVAL;
    }
    if (place_in_list >= LIST_LENGTH) {
      // We're out of instructions. Time to make some.
      for(int i = 0; i < LIST_LENGTH; i += 2) {
        // We're going to add instructions in pairs - either a double-and-half time pair or a pair of normals.
        // Adding the half and double speed in pairs - even if they're not done adjacently (as long as they *do* get done)
        // will insure the clock will keep long-term time accurately.
        switch(random(2)) {
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
      // Now shuffle the array
      for(int i = 0; i < LIST_LENGTH - 1; i++) {
        unsigned char swapspot = random(i, LIST_LENGTH);
        unsigned char temp = instruction_list[i];
        instruction_list[i] = instruction_list[swapspot];
        instruction_list[swapspot] = temp;
      }
      // This must be even!
      // It also should be long enough to establish a pattern
      // before changing.
      time_per_step = random(5) * 6 + 10;
      place_in_list = 0;
      time_in_step = 0;
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
#ifdef DEBUG
      // signal the transition point. Should happen once every "time per step" seconds
      digitalWrite(P_UNUSED, HIGH);
      delay_ms(30);
      digitalWrite(P_UNUSED, LOW);
#endif
      time_in_step = 0;
      place_in_list++;
    }
  }
}
