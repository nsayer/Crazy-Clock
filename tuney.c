/*

 Tuney Clock for Arduino
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
 * but will do so by interjecting various "song" patterns
 * into the mix
 *
 */

#if defined(UNIT_TEST)
// On *nix, there is no PROGMEM. Just make it go away and turn the
// pgm_read operations into just pointer derefs.
#define PROGMEM
#define pgm_read_byte(x) *(x)
#define pgm_read_ptr(x) ((void*)(*(x)))
#else
#include <avr/pgmspace.h>
#ifndef pgm_read_ptr
#define pgm_read_ptr pgm_read_word
#endif
#endif

#include "base.h"

// Each "song" is a series of pause counts - the pause between the ticks
// that make up the rhythm in question. To insure that the clock keeps
// proper time, the sum of the elements in each table must equal 9 times
// the number of elements in the table. The last element should be zero
// to mark the end.

// "Shave-and-a-haircut... two bits!"
PROGMEM const unsigned char shave_table[] = { 20, 4, 2, 2, 4, 8, 4, 28, 0 };
// Backbeat from "Heart Of Rock-n-Roll"
PROGMEM const unsigned char backbeat_table[] = { 24, 6, 2, 12, 6, 2, 12, 6, 2, 12, 6, 2, 25, 0 };
// SOS in morse
PROGMEM const unsigned char sos_table[] = { 23, 2, 2, 4, 6, 6, 8, 2, 2, 35, 0};

PROGMEM void* const song_table[] = { &shave_table, &backbeat_table, &sos_table };

#define SONG_COUNT 3

void loop() {
  while(1) {
    // Do this about once a minute-ish.
    if (q_random() % 30 != 0) {
      // a normal second.
      doTick();
      for(int i = 0; i < IRQS_PER_SECOND - 1; i++)
        doSleep();
      continue;
    }

    // Time to play a song!
    unsigned int song = q_random() % SONG_COUNT;
    unsigned char *current_song = (unsigned char*)pgm_read_ptr(song_table + song);
    while(1) {
      unsigned char song_data = pgm_read_byte(current_song++);
      if (song_data == 0) break; // song over
      doTick();
      for(int i = 0; i < song_data; i++)
        doSleep();
    }
  }
}
