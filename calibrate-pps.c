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
 * The code will set up timer 0 to count from the system clock, prescaled by 64.
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
 * very long time - almost 6 hours.
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
#define SECONDS_COUNT 19532
#define EXPECTED_COUNT 10000384

// And this is where we write the delta
#define EE_TRIM_LOC ((void*)4)

volatile unsigned long timer_high_bits = 0;

ISR(TIM0_OVF_vect) {
  timer_high_bits += 0x100;
}

static inline unsigned char read_input_pin() {
  return (PINB & _BV(PINB0))?1:0;
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

  sei(); // enable interrupts
 
  while(!read_input_pin()) ; // wait for a rising edge
  PORTB |= _BV(PB1); // light the LED
  unsigned char last_state = 1;
  unsigned int rising_edges = 0;

  TCCR0B = _BV(CS01) | _BV(CS00); // prescale = 64

  while(1) {
    unsigned char pin_state = read_input_pin();
    if (pin_state == last_state) continue;
    last_state = pin_state;
    if (pin_state == 0) continue; // we don't count falling edges

    rising_edges++;
    if (rising_edges == SECONDS_COUNT) break;
  }
  TCCR0B = 0; // stop the clock  
  
  timer_high_bits |= TCNT0;

  int delta = (int)(((long)timer_high_bits) - EXPECTED_COUNT);

  eeprom_write_word(EE_TRIM_LOC, delta);
  PORTB = 0; // Turn the LED off

  while(1); // and we're done.
}

