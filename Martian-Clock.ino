/*

 Lazy Clock for Arduino
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
 * This clock keeps martian Sol time. That is, each day is 2.75% longer.
 * We achieve this by adding 99 extra seconds per hour (actually 99 tenths every
 * six minutes). 
 *
 */
 
#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Turn this on to use the unused output as a debug output
// #define DEBUG

// We're going to set up a timer interrupt for every 100 msec, so what's 1/.1?
// Note that while we're actually doing stuff, we *must* insure that we never
// work through an interrupt. This is because we're not *counting* these
// interrupts, we're just waiting for each one in turn.
#define IRQS_PER_SECOND (10)

// clock solenoid pins
#define P0 0
#define P1 1
#define P_UNUSED 2

// How long is each tick? In this case, we're going to busy-wait on the timer.
#define TICK_LENGTH (35)

// This will alternate the ticks
#define TICK_PIN (lastTick == P0?P1:P0)

unsigned char lastTick;

// We need to add 99 extra sleeps every 6 minutes (3600 systicks).
// We do this evenly by doing 36 of them every 37 systicks, then
// 63 of them every 36 systicks. 36+63 = 99 and 36*37+36*63 = 3600. QED.
#define CYCLE_COUNT (99)
#define SHORT_CYCLE (36)
#define LONG_CYCLE (37)
#define NUM_LONG_CYCLES (36)

// This delay loop is magical because we know the timer is ticking at 500 Hz.
// So we just wait until it counts N/2 times and that will be an N msec delay.
static void delay_ms(unsigned char msec) {
   unsigned char start_time = TCNT0;
   while(TCNT0 - start_time < msec / 2) ; // sit-n-spin
}

// Each call to doTick() will "eat" a single one of our interrupt "ticks"
static void doTick() {
  digitalWrite(TICK_PIN, HIGH);
  delay_ms(TICK_LENGTH);
  digitalWrite(TICK_PIN, LOW);
  lastTick = TICK_PIN;
  sleep_mode(); // eat the rest of this tick
}

ISR(TIMER0_COMPA_vect) {
  // do nothing - just wake up
}

void setup() {
  clock_prescale_set(clock_div_8);
  power_adc_disable();
  power_usi_disable();
  power_timer1_disable();
  TCCR0A = _BV(WGM01); // mode 2 - CTC
  TCCR0B = _BV(CS02) | _BV(CS00); // prescale = 1024
  // xtal freq = 4.096 MHz.
  // CPU freq = 4.096 MHz / 8 = 512 kHz
  // count freq = 512 kHz / 1024 = 500 Hz
  OCR0A = 49; // 10 Hz - don't forget to subtract 1 - the counter is 0-49.
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
}

void loop() {
  unsigned int sol_counter = 0; // this counts to either 36 or 37 before we add an extra sleep
  unsigned int bump_counter = 0; // this counts bumps from 0 to CYCLE_LENGTH
  while(1) {
    for(int i = 0; i < IRQS_PER_SECOND; i++) {
      if (i == 0)
        doTick();
      else
        sleep_mode();
      unsigned char this_bump_length = (bump_counter < NUM_LONG_CYCLES)?LONG_CYCLE:SHORT_CYCLE;
      if (sol_counter++ >= this_bump_length) {
        sleep_mode(); // the extra sleep
        sol_counter = 0;
        if (bump_counter++ >= CYCLE_COUNT) {
          bump_counter = 0;
        }
      }
    }
  }
}