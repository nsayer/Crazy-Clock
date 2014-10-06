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

// Update the PRNG seed daily
#define SEED_UPDATE_INTERVAL (86400)

// We're going to set up a timer interrupt for every 100 msec, so what's 1/.1?
// Note that while we're actually doing stuff, we *must* insure that we never
// work through an interrupt. This is because we're not *counting* these
// interrupts, we're just waiting for each one in turn.
#define IRQS_PER_SECOND (10)

// You mark time by calling this method. It puts the CPU to sleep until
// the next timer interrupt. It will also do the funky math to adjust
// the interrupt counter to keep them happening at a nominal 10 Hz rate.
void doSleep();

// This method will tick the clock, and then call doSleep(). So in short,
// for the clock to keep proper time, you must call doSleep() 9 times
// for every call to doTick().
void doTick();

// If the clock uses random(), then every so often, it should call this
// method. A random seed is kept in EEPROM. Calling this method perturbs
// this seed so that every time you change the battery you don't see the
// same patterns.
void updateSeed();

