Crazy-Clock
===========

This is a lavet stepper motor driver sketch inspired (indirectly) by https://github.com/akafugu/vetinari_clock/

This repository contains several different sketches for the clock, each with a different algorithm.

The hardware uses either a 4.000 MHz or 32.768 kHz crystal as a timing source. In main(), for the 4 MHz crystal option the CPU clock prescaler is set to divide the clock down to as close to 32.768 kHz as we can get (obviously the 32 kHz crystal is already there, so no prescaling is set). That done, timer0 is prescaled by 64 and then set up in CTC mode. The resulting counting frequency is divided by 10 Hz. The result is never a whole number, so we set up a cycle with the fractional denominator number of interrupts. For the 0-numerator interrupt, set the CTC register to the quotient + 1. For the remainder of the cycles, we set the CTC register to the quotient without adding 1. The result of that will be a (nominal) 10 Hz interrupt source. For a 4.00 MHz crystal divided by 128 the system clock is 31.25 kHz. 31.25 kHz divided by 640 is 48 + 53/64. So that's 53 interrupts counting to 49 and 11 counting to 48. 53 * 49 + 48 * 11 = 3125. 125000/2560 = 48.828125. 48.8128125 * 3125 = 64. So every 6.4 seconds, our whacky 48.8128125 Hz counting rate syncs up with 10 Hz. The longer intervals will be 2.048 msec longer than the shorter ones, but for this application that's insignificant. The interrupts aren't in and of themselves going to be used, but they will wake up the CPU. sleep_mode() will be used, along with the wake-up, to mark time. And putting the cpu into idle (which is the most we can do and still keep the timer running) reduces current consumption down to less than 100 µA. For a 32.768 kHz crystal the 10 Hz dividing fraction is 51 1/5.

base.c/base.h form a support library, of sorts. The doSleep(), doTick() and updateSeed() methods are exported for the individual clock code to use. main() is also there and sets up the basic 10 Hz interrupt cycle. Once the hardware is set up, it calls loop() in a while-forever.

crazy.c is the Crazy Clock. It builds random instruction lists consisting of pairs of intervals of slow ticking and fast ticking, along with intervals of normal ticking. The intention is that a single period of slow ticking paired with a period of fast ticking will net the correct number of ticks.


lazy.c is the Lazy Clock. It is just stopped most of the time. It does all of its ticking quickly and all at once, then "rests."


vetinari.c is the Vetinari Clock. It stealthily and randomly inserts extra tenths of a second between ticks and once it's gathered up ten of them, performs a special "stutter" tick, ticking twice in rapid succession, ruining the natural rhythm of the clock.


whacky.c is the Whacky Clock. It ticks once per second, but on a different tenth-of-a-second for each. It gives off the vibe of a stumbling drunk.


wavy.c is the Wavy Clock. It's tick frequency is proportional to a sine wave. It's sort of... surgy...


warpy.c is the warpy clock. It ticks 10% faster for 12 hours, and then 10% slower for 12 hours. Makes the days FLY by.


drift.h is a common infrastructure for clocks which tick simply and at a constant rate, but at a rate different than 86400 ticks per day. For such clocks, the expectation is that they will define a fraction similar to how the 10 Hz clock is generated. drift.h will use that fraction to either add or remove calls to doSleep() evenly across time. The result will be a clock that runs a fixed and accurate amount fast or slow relative to SI time (86400 seconds per day).

The Martian clock ticks in Martian Sols. A day is 24 hours, 39 minutes, 36 seconds.


The Sidereal clock keeps Sidereal (astronomical) time. A day is 23 hours, 56 minutes, 4 seconds.


The Tidal clock keeps lunar tidal time. A day is 24 hours, 50 minutes, 28 seconds.


There is a normal clock as well. It's useful for testing, or if you modify a clock as a joke, but then want to put it back to normal. Since the installation procedure is generally destructive (it's a lot like a heart transplant: you generally can't make the old one work ever again when you're done), it's much easier to simply reprogram the new controller to be boring.


This version no longer uses the Arduino IDE. It's just built with the AVR toolchain. The makefile's main target is 'init.' You call it with TYPE set to the type of clock you want. The init rule will set the fuses, upload the flash, and write 4 random bytes to the start of EEPROM to be used as a PRNG seed. Note that you must edit both the 'base.c' file and the Makefile to configure the correct crystal options.

