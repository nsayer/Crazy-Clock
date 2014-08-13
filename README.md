Crazy-Clock
===========

This is a lavet stepper motor driver sketch inspired (indirectly) by https://github.com/akafugu/vetinari_clock/

This repository contains several different sketches for the clock, each with a different algorithm.

The initial hardware version uses a 4.096 MHz crystal as a timing source. setup() is the same for each - the CPU clock prescaler is set to divide whatever crystal clock is present down to 512 kHz. That done, timer0 is prescaled by 1024 and then set up in CTC mode to be a 10 Hz interrupt source. The interrupts aren't in and of themselves going to be used, but they will wake up the CPU. sleep_mode() will be used, along with the wake-up, to mark time.

The crazy clock is the initial version. It is the most subtle algorithm. It builds random instruction lists consisting of pairs of intervals of slow ticking and fast ticking, along with intervals of normal ticking. The intention is that a single period of slow ticking paired with a period of fast ticking will net the correct number of ticks.

The lazy clock is just stopped most of the time. It does all of its ticking quickly and all at once, then "rests."

The whacky clock ticks once per second, but on a different tenth-of-a-second for each. It gives off the vibe of a stumbling drunk.

The wavy clock's tick frequency is proportional to a sine wave. It's sort of... surgy...

Lord Vetinari would likely approve.
