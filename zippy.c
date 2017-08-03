/*

 Zippy Clock
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
 * This clock just keeps regular time time, but at a fixed multiple/divisor
 * of normal time.
 *
 * PAUSE_TICKS is the number of pause beats (tenths of a second)
 * between ticks.
 *
 * 49  80% slower (1/5 speed)
 * 39  75% slower (1/4 speed)
 * 29  33% slower (1/3 speed)
 * 19  50% slower (half speed)
 * 18  47.3% slower
 * 17  44.4% slower
 * 16  41.1% slower
 * 15  37.5% slower
 * 14  33.3% slower (2/3 speed)
 * 13  29% slower
 * 12  23% slower
 * 11  17% slower
 * 10  10% slower
 * 9   Normal
 * 8   11.1% faster
 * 7   25% faster
 * 6   42.8% faster
 * 5   66.6% faster
 * 4   100% faster (2x speed)
 * 3   150% faster (2.5x speed)
 * 2   233.3% faster (3.33x speed)
 * 1   5x faster
 * 0   10x faster
 */

#define PAUSE_TICKS 1

#include "base.h"

void loop() {
  while(1){
    doTick();
    for(unsigned char i = 0; i < PAUSE_TICKS; i++)
      doSleep();
  }
}
