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
 * This is a calibration sketch. It's intended to connect the system clock as directly
 * as possible to an I/O pin. The best we can do is generate a half-speed clock by
 * telling timer0 to toggle OC0A with the system clock. The result is 16.384 kHz.
 */

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/power.h>

void main() {
  ADCSRA = 0; // DIE, ADC!!! DIE!!!
  ACSR = _BV(ACD); // Turn off analog comparator - but was it ever on anyway?
  power_adc_disable();
  power_usi_disable();
#ifdef __AVR_ATtiny44__
  power_timer0_disable();
  TCCR1A = _BV(COM1A0); // toggle OC1A
  TCCR1B = _BV(WGM12) | _BV(CS10); // mode 4 - CTC, prescale = 1 (none)
  OCR1A = 0; // as fast as possible
  DDRA = 0xff; // All unused pins are output
  DDRB = _BV(DDB2);
  PORTA = 0; // Initialize all pins low.
  PORTB = 0; // Initialize all pins low.
#else
  power_timer1_disable();
  TCCR0A = _BV(COM0A0) | _BV(WGM01); // mode 2 - CTC, toggle OC0A
  TCCR0B = _BV(CS00); // prescale = 1 (none)
  OCR0A = 0; // as fast as possible
  DDRB = _BV(DDB0) | _BV(DDB1) | _BV(DDB2); // all unused pins are output
  PORTB = 0; // Initialize all pins low.
#endif

  // And we're done.
  while(1);

}

