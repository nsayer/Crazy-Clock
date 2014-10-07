Crazy-Clock
===========

This is a lavet stepper motor driver sketch inspired (indirectly) by https://github.com/akafugu/vetinari_clock/

This repository contains several different sketches for the clock, each with a different algorithm.

The initial hardware version uses a 4.096 MHz or 4.000 MHz crystal as a timing source. setup() is the same for each - the CPU clock prescaler is set to divide whatever crystal clock is present down to as close to 32.768 kHz as we can get. That done, timer0 is prescaled by 64 and then set up in CTC mode. The resulting counting frequency is divided by 10 Hz. If the result is not a whole number, then we'll set up a cycle with the fractional denominator number of interrupts. For the 0-numerator interrupt, set the CTC register to the quotient + 1. For the remainder of the cycles, we set the CTC register to the quotient without adding 1. The result of that will be a (nominal) 10 Hz interrupt source. For example, a 4.00 MHz crystal divided by 128 is 31.25 kHz. 31.25 kHz divided by 640 is 48 + 53/64. So that's 53 interrupts counting to 49 and 11 counting to 48. 53 * 49 + 48 * 11 = 3125. 125000/2560 = 48.828125. 48.8128125 * 3125 = 64. So every 6.4 seconds, our whacky 48.8128125 Hz counting rate syncs up with 10 Hz. The longer intervals will be 2.048 msec longer than the shorter ones, but for this application that's insignificant. The interrupts aren't in and of themselves going to be used, but they will wake up the CPU. sleep_mode() will be used, along with the wake-up, to mark time. And putting the cpu into idle (which is the most we can do and still keep the timer running) reduces current consumption down to less than 100 µA.

You can also configure the hardware with a 32.768 kHz crystal. In this case, no system clock prescaling occurs at all, the timer prescaler is set to 64 and the 10 Hz dividing fraction is 51 1/5.

The crazy clock is the initial version. It builds random instruction lists consisting of pairs of intervals of slow ticking and fast ticking, along with intervals of normal ticking. The intention is that a single period of slow ticking paired with a period of fast ticking will net the correct number of ticks.


The lazy clock is just stopped most of the time. It does all of its ticking quickly and all at once, then "rests."


The Vetinari clock stealthily and randomly inserts extra tenths of a second between ticks and once it's gathered up ten of them, performs a special "stutter" tick, ticking twice in rapid succession, ruining the natural rhythm of the clock.


The whacky clock ticks once per second, but on a different tenth-of-a-second for each. It gives off the vibe of a stumbling drunk.


The wavy clock's tick frequency is proportional to a sine wave. It's sort of... surgy...


The warpy clock ticks 10% faster for 12 hours, and then 10% slower for 12 hours. Makes the days FLY by.


The Martian clock ticks in Martian Sols. A day is 24 hours, 39 minutes, 36 seconds.


The Sidereal clock keeps Sidereal (astronomical) time. A day is 23 hours, 56 minutes, 4 seconds.


There is a normal clock as well. It's useful if you modify a clock as a joke, but then want to put it back to normal. Since the installation procedure is generally destructive (it's a lot like a heart transplant: you generally can't make the old one work ever again when you're done), it's much easier to simply reprogram the new controller to be boring.


This version no longer uses the Arduino IDE. It's just built with the AVR toolchain. The makefile's main target is 'init.' You call it with TYPE set to the type of clock you want. The init rule will set the fuses, upload the flash, and write 4 random bytes to the start of EEPROM to be used as a PRNG seed. Note that you must edit both the 'base.c' file and the Makefile to configure the correct crystal options.

