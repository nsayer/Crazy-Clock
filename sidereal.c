/*

 Sidereal Clock for Arduino
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

#include "base.h"

// We need to remove 4 minutes 56 seconds every day.
// That fraction is 366 + 6/59.
// This is the whole number
#define BASE_CYCLE_LENGTH (366)
// This is the fractional numerator
#define NUM_LONG_CYCLES (6)
// This is the fractional denominator
#define CYCLE_COUNT (59)
// To make the clock run a fraction slower rather than faster, uncomment this.
//#define RUN_SLOW

#include "drift.h"

