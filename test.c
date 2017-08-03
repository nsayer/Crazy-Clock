/*

 Crazy Clock test suite
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
 * This file is a test harness for the individual clock codes.
 *
 * It's intended to be compiled on a *nix system. It will emulate
 * the hardware by simply printing out "Sleep" or "Tick" for each
 * call, as appropriate.
 *
 * The purpose is to insure that the correct ticking frequency is
 * maintained. Turn a single clock file and this code into a *nix
 * executable and pipe the output into "head -864000 | sort | uniq -c"
 * and you should see "86400 Tick", meaning that out of 864000
 * tenths-of-a-second, the clock ticked 10% of the time. That's a
 * clock that's running on-time.
 *
 * For drifting clocks, the procedure is slightly different. You
 * add or subtract the appropriate amount of time so that you get
 * an interval of time over which you expect the clock to tick
 * 86400 times - that is, the interval of real time that represents
 * one "day" on the clock. Work that out in the number of tenths of
 * a second and use that as the argument to 'head' above.
 * The result should still be '86400 Tick'.
 *
 * The random clocks may not print out '86400 Tick' every time, because
 * they may wind up in the middle of a transition point at the day boundary.
 * But done enough times, the average should be 86400. You can also
 * try to minimize the impact by multiplying 864000 by some number of
 * days. If you don't get 86400*days Tick lines, then it should at least
 * be extremely close.
 */

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

extern void loop();

unsigned int q_random() {
  return random();
}

void doSleep() {
  printf("Sleep\n");
}

void doTick() {
  printf("Tick\n");
}

int main(int argc, char **argv) {
  srandom(time(NULL));
  while(1) loop();
}
