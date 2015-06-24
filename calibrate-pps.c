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
 * This is a calibration sketch. It's purpose is to figure out the correct
 * compensation drift given a reference pulse-per-second input, such as from
 * a GPS receiver.
 *
 * The code will set up timer 0 to count from the system clock, prescaled by 8.
 * An overflow interrupt will be used to extend the 16 bit timer counter
 * to 32 bits.
 *
 * Once the correct number of rising edges (after the first) have occurred, the timer
 * will be stopped and read. The expected count will be subtracted from the
 * actual count and the result used as the drift factor and written into
 * EEPROM. At that point, the other clock pin will be set high, and can be used
 * to turn on an LED to indicate completion.
 *
 * We have to prescale the system clock because otherwise executing code will take
 * some number of ticks and throw things off. We still may wind up off by one or two
 * counts, and in order to minimize that impact, we need to run the test for a very,
 * very long time - almost an hour.
 *
 * Connect PB0 to a PPS source and connect PB1 to the anode of an LED (no series
 * resistor required if you use the clock pins). The clock must be powered by 3
 * volts or less. The LED will light when the timing interval begins and extinguish
 * when it's over. When the LED goes out, you can upload the clock code and go.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include <avr/power.h>

// in this many seconds, we expect to get this many counts
// The value should be result in an EXPECTED_COUNT of around 10M for best results.
#define SECONDS_COUNT (2442)
#define EXPECTED_COUNT (4096LL * SECONDS_COUNT)

// And this is where we write the delta
#define EE_TRIM_LOC ((void*)4)

volatile unsigned long second_counter = 0;

ISR(PCINT0_vect) {
  // We're using the pin change interrupt, so we need to count
  // rising *and* falling edges
  // We must do this as quickly as possible
  if (++second_counter == 2 * SECONDS_COUNT)
    TCCR0B = 0; // stop the clock
}

volatile unsigned long timer_high_bits = 0;

ISR(TIM0_OVF_vect) {
  timer_high_bits += 0x100;
}

void main() {
  ADCSRA = 0; // DIE, ADC!!! DIE!!!
  ACSR = _BV(ACD); // Turn off analog comparator - but was it ever on anyway?
  power_adc_disable();
  power_usi_disable();
  power_timer1_disable();
  TCCR0A = 0; // normal mode - count to 0xff and repeat
  TCCR0B = 0; // prescale = 0: stop the clock.
  TCNT0 = 0; // zero the timer
  TIMSK = _BV(TOIE0); // overflow interrupt only
  DDRB = _BV(DDB1) | _BV(DDB2); // PB0 is input, the rest output.
  PORTB = 0; // Initialize all pins low.
  PCMSK = _BV(PCINT0); // PB0 is our PC interrupt source
  GIMSK = _BV(PCIE); // turn on PPS interrupt
  sei(); // enable interrupts

  for(second_counter = 0; second_counter < 20;); // wait for the interrupt system to count 10 seconds
  // Do this quickly
  TCCR0B = _BV(CS01); // prescale = 8 - turn on the clock
 
  PORTB |= _BV(PB1); // light the LED

  second_counter = 0; 
  while(TCCR0B != 0) ; // wait until the interrupt system stops the clock
  GIMSK = 0; // disable the PC interrupt - we're done with it.

  // Fill in the low bits from the timer counter
  timer_high_bits |= TCNT0;

  // debug - write the actual count
  eeprom_write_dword((void*)8, timer_high_bits);

  // how far off are we from the expected count?
  long long delta = (((long long)timer_high_bits) - EXPECTED_COUNT);

  // calculate the drift value - tenths of a ppm
  int ppm = (int)((delta * 10000000LL) / EXPECTED_COUNT);

  eeprom_write_word(EE_TRIM_LOC, ppm);
  PORTB = 0; // Turn the LED off

  while(1); // and we're done.
}

