/*

 Lunar Sidereal Clock
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

// This clock does 12 hours worth of ticking over a sidereal month.
// That is, the time it takes the moon to return to the same position
// relative to the stars.

// This clock needs 12 hours worth of ticking in 27 days, 7:43:11.6.

#define WHOLE 545
#define NUMERATOR 4679
#define DENOMINATOR 10800

#include "slow.h"
