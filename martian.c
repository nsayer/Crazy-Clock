/*

 Martian Clock for Arduino
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

// We need to add 39 minutes 36 seconds every day. That's means the
// fraction is 36 + 12/33.
// This is the whole number
#define BASE_CYCLE_LENGTH (36)
// This is the fractional numerator
#define NUM_LONG_CYCLES (12)
// This is the fractional denominator
#define CYCLE_COUNT (33)
// To make the clock run a fraction slower rather than faster, uncomment this.
// This is for the martian clock.
#define RUN_SLOW

#include "drift.h"

