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
 * This is intended to run on an ATTiny45. Connect a 32.768 kHz crystal and fuse
 * it for the low-frequency oscillator, no watchdog or brown-out detector.
 *
 * Connect PB0 and PB1 to the coil pins of a Lavet stepper coil of a clock movement
 * (with a series resistor and flyback diode to ground on each pin) and power it 
 * from a 3.3 volt boost converter.
 *
 * This file is the common infrastructure for all of the different clock types.
 * It sets up a 10 Hz interrupt. The clock code(s) keep accurate time by calling
 * either doTick() or doSleep() repeatedly. Each method will put the CPU to sleep
 * until the next tenth-of-a-second interrupt (doTick() will tick the clock once first).
 * In addition, doTick() and doSleep(), will occasionally (SEED_UPDATE_INTERVAL)
 * write out the PRNG seed (if it's changed) to EEPROM. This will insure that the clock
 * doesn't repeat its previous behavior every time you change the battery.
 *
 * The clock code should insure that it doesn't do so much work that works through
 * a 10 Hz interrupt interval. Every time that happens, the clock loses a tenth of
 * a second. In particular, generating random numbers is a costly operation.
 *
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <stdlib.h>

#include "base.h"

#if !defined(__AVR_ATtiny44__) && !defined(__AVR_ATtiny45__)
#error Unsupported chip
#endif

// 32,768 divided by (64 * 10) yields a divisor of 51 1/5, which is 52 + 51*4
#define CLOCK_CYCLES (5)
// Don't forget to decrement the OCR0A value - it's 0 based and inclusive
#define CLOCK_BASIC_CYCLE (51 - 1)
// a "long" cycle is CLOCK_BASIC_CYCLE + 1
#define CLOCK_NUM_LONG_CYCLES (1)

// One day in tenths-of-a-second
#define SEED_UPDATE_INTERVAL 864000L
#define EE_PRNG_SEED_LOC ((void*)0)
#define EE_TRIM_LOC ((void*)4)

// clock solenoid pins
#ifdef __AVR_ATtiny44__
#define CLOCK_PORT PORTA
#define P0 PORTA5
#define P1 PORTA6
#define CLOCK_DDR DDRA
// Our two clock pins are outputs, the rest don't matter.
#define CLOCK_DDR_BITS (_BV(DDA5) | _BV(DDA6))
#else
#define CLOCK_PORT PORTB
#define P0 PORTB0
#define P1 PORTB1
#define CLOCK_DDR DDRB
#define CLOCK_DDR_BITS (_BV(DDB0) | _BV(DDB1))
#endif

// For a 32 kHz system clock speed, random() is too slow.
// Found this at http://uzebox.org/forums/viewtopic.php?f=3&t=250
static long seed;
#define M (0x7fffffffL)

unsigned long q_random() {
  seed = (seed >> 16) + ((seed << 15) & M) - (seed >> 21) - ((seed << 10) & M);
  if (seed < 0) seed += M;
  return (unsigned long) seed;
}

static void updateSeed() {
  // Don't bother exercising the eeprom if the seed hasn't changed
  // since last time.
  eeprom_update_dword(EE_PRNG_SEED_LOC, seed);
}

volatile static unsigned char sleep_miss_counter = 0;

volatile unsigned long trim_cycles;
volatile char trim_offset;

static unsigned long seed_update_timer;

void doSleep() {

  if (--seed_update_timer == 0) {
    updateSeed();
    seed_update_timer = SEED_UPDATE_INTERVAL;
  }

  // If we missed a sleep, then try and catch up by *not* sleeping.
  // Note that the test-and-decrememnt must be atomic, so save a
  // copy of the present value before decrementing and use that
  // copy for the decision.
  unsigned char local_smc;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    local_smc = sleep_miss_counter--;
  }
  if (local_smc == 0)
    sleep_mode(); // this results in sleep_miss_counter being incremented.
}

// How long is each tick pulse?
#define TICK_LENGTH (35)

// This will alternate the ticks
#define TICK_PIN (lastTick == P0?P1:P0)

// Each call to doTick() will "eat" a single one of our interrupt "ticks"
void doTick() {
  static unsigned char lastTick; // Doesn't matter that it's uninitialized.

  CLOCK_PORT |= _BV(TICK_PIN);
  _delay_ms(TICK_LENGTH);
  CLOCK_PORT &= ~ _BV(TICK_PIN);
  lastTick = TICK_PIN;
  doSleep(); // eat the rest of this tick
}

ISR(TIM0_COMPA_vect) {
  static unsigned char cycle_pos = 0;
  static unsigned long trim_pos = 0;

  char offset = 0;
  if (trim_offset != 0) {
    // This is how many crystal cycles we just went through.
    unsigned long crystal_cycles = OCR0A;
    if (trim_pos < crystal_cycles) {
      trim_pos += trim_cycles; // how often do we nudge by 1 unit?
      offset = trim_offset; // which direction?
    }
    trim_pos -= crystal_cycles;
  }

  // This is the magic for fractional counting.
  // Alternate between adding an extra count and
  // not adding one. This means that the intervals
  // are not uniform, but it's only by 2 ms or so,
  // which won't be noticable for this application.
  if (++cycle_pos >= CLOCK_CYCLES) cycle_pos = 0;

  // because offset will change from 0 to +/- 1 for one cycle,
  // that means we have to set OCR0A *every* time.
  if (cycle_pos >= CLOCK_NUM_LONG_CYCLES)
    OCR0A = ((char)CLOCK_BASIC_CYCLE) + offset;
  else
    OCR0A = ((char)(CLOCK_BASIC_CYCLE + 1)) + offset;

  // Keep track of any interrupts we blew through.
  // Every increment here *should* be matched by
  // a decrement in doSleep();
  sleep_miss_counter++;
}

extern void loop();

// main() is void, and we never return from it.
void __ATTR_NORETURN__ main() {
  ADCSRA = 0; // DIE, ADC!!! DIE!!!
  ACSR = _BV(ACD); // Turn off analog comparator - but was it ever on anyway?
  power_adc_disable();
  power_usi_disable();
  power_timer1_disable();
  TCCR0A = _BV(WGM01); // mode 2 - CTC
  TCCR0B = _BV(CS01) | _BV(CS00); // prescale = 64
#ifdef __AVR_ATtiny44__
  TIMSK0 = _BV(OCIE0A); // OCR0A interrupt only.
#else
  TIMSK = _BV(OCIE0A); // OCR0A interrupt only.
#endif
  
  set_sleep_mode(SLEEP_MODE_IDLE);

  CLOCK_DDR = CLOCK_DDR_BITS;
  CLOCK_PORT = 0; // Initialize all pins low.

  // we pre-compute all of this stuff to save cycles later.
  // These values never change after startup.
  // The uninitialized value of 0xffff is actually rather harmless.
  // It's the signed int -1, which speeds up the clock by 0.1 ppm.
  int trim_value = (int)eeprom_read_word(EE_TRIM_LOC);
  if (trim_value != 0) {
    trim_cycles = 10000000 / abs(trim_value); // how often do we nudge by 1 unit?
    trim_offset = (trim_value < 0)?-1:1; // signum - which direction?
  } else
    trim_offset = 0;

  // Try and perturb the PRNG as best as we can
  seed = (long)eeprom_read_dword(EE_PRNG_SEED_LOC);
  // it can't be all 0 or all 1
  if (seed == 0 || ((seed & M) == M)) seed=0x12345678L;
  q_random(); // perturb it once...
  updateSeed(); // and write it back out - a new seed every battery change.

  // initialize this so it doesn't have to be in the data segment.
  seed_update_timer = SEED_UPDATE_INTERVAL;

  // Set up the initial state of the timer.
  OCR0A = CLOCK_BASIC_CYCLE + 1;
  TCNT0 = 0;

  // Don't forget to turn the interrupts on.
  sei();

  // Now hand off to the specific clock code
  while(1) loop();
  __builtin_unreachable();

}

