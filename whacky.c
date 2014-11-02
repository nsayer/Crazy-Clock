/*

 Whacky Clock for Arduino
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
 * but will do so by picking a random tenth-of-a-second within each second for the ticking.
 *
 */

#include "base.h"

void loop() {
  while(1){
    unsigned char tick_position = q_random() % IRQS_PER_SECOND; //0-9, inclusive

    for(unsigned char i = 0; i < IRQS_PER_SECOND; i++)
      if (i == tick_position)
        doTick();
      else
        doSleep();
  }
}
