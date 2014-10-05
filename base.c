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

#include <stdlib.h> 
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#include "base.h"

// Our design choices may wind up with a system clock of either 125 kHz
// or 128 kHz. If it's 125 kHz, then we have to do some juggling to wind up
// with the proper IRQS_PER_SECOND value of 10. To set that up, uncomment
// this:
#define TEN_BASED_CLOCK

// Another option is the 32 kHz clock crystal. For that one, we need to not
// set up a sysclock prescaler, change the delay_ms() timing, use a smaller
// timer prescaler, and use a different fraction for the interrupt cycle timing.
//#define THIRTYTWO_KHZ_CLOCK

#if defined(TEN_BASED_CLOCK) && defined(THIRTYTWO_KHZ_CLOCK)
#error Must pick either 10 based, 32 kHz or neither.
#endif

#if defined(TEN_BASED_CLOCK)
// 125,000 divided by (256 * 10) is a divisor of 48 53/64, which is 49*53 + 48*11
#define CLOCK_CYCLES (64)
// Don't forget to decrement the OCR0A value - it's 0 based and inclusive
#define CLOCK_BASIC_CYCLE (48 - 1)
// a "long" cycle is CLOCK_BASIC_CYCLE + 1
#define CLOCK_NUM_LONG_CYCLES (53)
#elif defined(THIRTYTWO_KHZ_CLOCK)
// 32768 divided by (256 * 10) yields a divisor of 12 4/5, which is 13*4 + 12
#define CLOCK_CYCLES (5)
// Don't forget to decrement the OCR0A value - it's 0 based and inclusive
#define CLOCK_BASIC_CYCLE (12 - 1)
// a "long" cycle is CLOCK_BASIC_CYCLE + 1
#define CLOCK_NUM_LONG_CYCLES (4)
#endif

// clock solenoid pins
#define P0 0
#define P1 1
#define P_UNUSED 2

// How long is each tick? In this case, we're going to busy-wait on the timer.
#define TICK_LENGTH (35)

static void delay_ms(unsigned char msec) {
   unsigned char start_time = TCNT0;
#ifdef THIRTYTWO_KHZ_CLOCK
// For the 32 kHz clock, the counting rate is 128 Hz. This means that our
// granularity is 7.8125 msec, but this is not really a critical timing
// interval.
   while(TCNT0 - start_time < (msec * 128L) / 1000) ; // sit-n-spin
#else
// This delay loop is magical because we know the timer is ticking at 500 Hz.
// So we just wait until it counts N/2 times and that will be an N msec delay.
// This will be a little off for TEN_BASED_CLOCK, but again, this is not a critical
// timing interval.
   while(TCNT0 - start_time < msec / 2) ; // sit-n-spin
#endif
}

void doSleep() {
#if defined(TEN_BASED_CLOCK) || defined(THIRTYTWO_KHZ_CLOCK)
  static unsigned char cycle_pos = 0xfe; // force a reset

  if (++cycle_pos == CLOCK_NUM_LONG_CYCLES)
    OCR0A = CLOCK_BASIC_CYCLE;
  if (cycle_pos >= CLOCK_CYCLES) {
    OCR0A = CLOCK_BASIC_CYCLE + 1;
    cycle_pos = 0;
  }
#endif
  sleep_mode();
}

// This will alternate the ticks
#define TICK_PIN (lastTick == P0?P1:P0)

// Each call to doTick() will "eat" a single one of our interrupt "ticks"
void doTick() {
  static unsigned char lastTick = P0;

  PORTB |= 1<<TICK_PIN;
  delay_ms(TICK_LENGTH);
  PORTB &= ~(1<<TICK_PIN);
  lastTick = TICK_PIN;
  doSleep(); // eat the rest of this tick
}

void updateSeed() {
  unsigned long seed = random();
  eeprom_write_dword(0, seed);
}

ISR(TIMER0_COMPA_vect) {
  // do nothing - just wake up
  _NOP();
}

extern void loop();

void main() {
#ifndef THIRTYTWO_KHZ_CLOCK
  // change this so that we wind up with a 125 kHz or a 128 kHz CPU clock.
  // And if it's 128 kHz, be sure to uncomment TEN_BASED_CLOCK above.
  clock_prescale_set(clock_div_32);
#endif
  ADCSRA = 0; // DIE, ADC!!! DIE!!!
  ACSR = _BV(ACD); // Turn off analog comparator - but was it ever on anyway?
  power_adc_disable();
  power_usi_disable();
  power_timer1_disable();
  TCCR0A = _BV(WGM01); // mode 2 - CTC
  TCCR0B = _BV(CS02); // prescale = 256
#if !defined(TEN_BASED_CLOCK) && !defined(THIRTYTWO_KHZ_CLOCK)
  // count freq = 128 kHz / 256 = 500 Hz
  OCR0A = 49; // 10 Hz - don't forget to subtract 1 - the counter is 0-49.
#endif
  TIMSK = _BV(OCIE0A); // OCR0A interrupt only.
  
  set_sleep_mode(SLEEP_MODE_IDLE);

  DDRB = (1<<P0) | (1<<P1) | (1<<P_UNUSED); // all our pins are output.
  PORTB = 0; // Initialize all pins low.
      
  // Try and perturb the PRNG as best as we can
  unsigned long seed = eeprom_read_dword(0);
  srandom(seed);
  updateSeed();

  sei();

  // Now hand off to the specific clock code
  while(1) loop();

}

