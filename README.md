Crazy-Clock
===========

This is a lavet stepper motor driver sketch inspired (indirectly) by https://github.com/akafugu/vetinari_clock/

This repository contains several different sketches for the clock, each with a different algorithm.

The hardware uses a 32.768 kHz crystal as a timing source. Timer0 is prescaled by 64 and then set up in CTC mode. The resulting counting frequency is divided by 10 Hz. The result is never a whole number, so we set up a cycle with the fractional denominator number of interrupts. For the 0-numerator interrupt, set the CTC register to the quotient + 1. For the remainder of the cycles, we set the CTC register to the quotient without adding 1. The result of that will be a (nominal) 10 Hz interrupt source. The 32.768 kHz crystal divided by 640 is 51 + 1/5. So that's 1 interrupt counting to 52 and 4 counting to 51. 52 + 51 * 4 = 256. 32768/256 = 128, with zero remainder. The longer intervals will be 2.048 msec longer than the shorter ones, but for this application that's insignificant. The interrupts aren't in and of themselves going to be used, but they will wake up the CPU. sleep_mode() will be used, along with the wake-up, to mark time. And putting the cpu into idle (which is the most we can do and still keep the timer running) reduces current consumption down to less than 100 µA.

If desired, there is a SW_TRIM option that will apply a corrective offset to the clock. The two bytes at addresses 4-5 of the EEPROM are the value, as a signed 16 bit value in tenths-of-a-ppm. Positive values slow the clock down. To figure out how far off the crystal is oscillating, it's necessary to generate an output clock signal that's related to the system clock. Attempting to read the crystal directly will affect the loading, changing the results. The best we can do is configure one of the timers to toggle one of the output lines at the system clock rate. The result is a nominal 16.384 kHz square wave. Measuring that with a frequency counter that's referenced from a GPS disciplined oscillator will result in a difference from nominal, which can be divided into the nominal frequency to get the error. Multiply the error by ten million to get the tenth-of-a-ppm value and that's the trim factor. calibrate.c is a firmware load that will generate the 16.384 kHz output for comparison and calibration.

Since the system clock is so slow, the libc random() function isn't usable. Instead, q_random() is supplied, which is a PRNG built with only addition and bit shifting. The first four bytes of EEPROM are a stored seed. The seed is saved daily (but only if it's used), and perturbed every time the battery is changed. The goal is to insure that the clock avoids any patterns as best as it can.

base.c/base.h form a support library, of sorts. The doSleep(), doTick() and q_random() methods are exported for the individual clock code to use. main() is also there and sets up the basic 10 Hz interrupt cycle, trimmed by the EEPROM trim factor. Once the hardware is set up, it calls loop() in a while-forever.

crazy.c is the Crazy Clock. It builds random instruction lists consisting of pairs of intervals of slow ticking and fast ticking, along with intervals of normal ticking. The intention is that a single period of slow ticking paired with a period of fast ticking will net the correct number of ticks.


lazy.c is the Lazy Clock. It is just stopped most of the time. It does all of its ticking quickly and all at once, then "rests."


vetinari.c is the Vetinari Clock. It stealthily and randomly inserts extra tenths of a second between ticks and once it's gathered up ten of them, performs a special "stutter" tick, ticking twice in rapid succession, ruining the natural rhythm of the clock.


whacky.c is the Whacky Clock. It ticks once per second, but on a different tenth-of-a-second for each. It gives off the vibe of a stumbling drunk.


wavy.c is the Wavy Clock. It's tick frequency is proportional to a sine wave. It's sort of... surgy...


warpy.c is the warpy clock. It ticks 10% faster for 12 hours, and then 10% slower for 12 hours. Makes the days FLY by.

tuny.c is the Tuney clock. It interrupts regular ticking periodically to tick out "songs" - particular rhythm patterns that should be familiar.

early.c is the Early clock. It's designed for people who like to set their clock ahead in order to be on-time. The early clock will stay anywhere between 0 and 10 minutes ahead, drifting back and forth. This prevents you from knowing exactly how far off it is, and compensating.


drift.h is a common infrastructure for clocks which tick simply and at a constant rate, but at a rate different than 86400 ticks per day. For such clocks, the expectation is that they will define a fraction similar to how the 10 Hz clock is generated. drift.h will use that fraction to either add or remove calls to doSleep() evenly across time. The result will be a clock that runs a fixed and accurate amount fast or slow relative to SI time (86400 seconds per day).

The Martian clock ticks in Martian Sols. A day is 24 hours, 39 minutes, 36 seconds.


The Sidereal clock keeps Sidereal (astronomical) time. A day is 23 hours, 56 minutes, 4 seconds.


The Tidal clock keeps lunar tidal time. A day is 24 hours, 50 minutes, 28 seconds.


There is a normal clock as well. It's useful for testing, or if you modify a clock as a joke, but then want to put it back to normal. Since the installation procedure is generally destructive (it's a lot like a heart transplant: you generally can't make the old one work ever again when you're done), it's much easier to simply reprogram the new controller to be boring.


This version no longer uses the Arduino IDE. It's just built with the AVR toolchain. The makefile has 3 main functions. 'fuse' will set the fuses as appropriate. Resetting the fuses on a working controller is *not* recommended. It should be done only once on any given controller. 'flash' will compile and upload the sketch indicated by the 'TYPE' macro. 'seed' will upload a 4 byte random seed to EEPROM. 'init' is an alias for 'fuse flash seed', but with the caveat that repeating 'fuse' is, again, *not* recommended. "init" is intended for bootstraping newly manufactured controllers.
