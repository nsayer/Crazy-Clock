/*

 Wavy Clock for Arduino
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
 * This code will keep a long-term average pulse rate of 1 Hz,
 * but will do so by tick at a speed proportional to a sine wave.
 *
 */

#include <avr/pgmspace.h>
#include "base.h"

PROGMEM const unsigned char sin_table[] = {9, 12, 15, 17, 18, 18, 17, 16, 13, 10, 8, 5, 2, 1, 0, 0, 1, 3, 6};

void loop() {
  for(int i = 0; i < sizeof(sin_table) / sizeof(unsigned char); i++) {
    doTick();
    for(int j = 0; j < pgm_read_byte(sin_table + i); j++)
      doSleep();
  }
}
