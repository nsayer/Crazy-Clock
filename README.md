Crazy-Clock
===========

This is a lavet stepper motor driver sketch inspired (indirectly) by https://github.com/akafugu/vetinari_clock/

This version changes the algorithm somewhat with the aim of appearing to be a much smoother operation.

The initial hardware version uses a 16.384 MHz crystal divided down to a final interrupt source of 10 Hz. The
sketch uses sleep_mode() to mark time.

The chip can tick one of three ways - half-time, normal time or double-time. It works by randomly creating an instruction list.
The list is filled with pairs of instructions - either two "normal tick" instructions or a "double time" and "half time" pair.
The list is then shuffled. Because of how the list is constructed, you can be sure that when the list is complete, the clock
will still be on-time.

To try and insure the clock is somewhat less... obvious... Each instruction in the list is allowed to run for anywhere from
ten to 30 seconds (but always an even number, and always the same length for a single list-construction).

Observers of the clock will see it speed up or slow down seemingly at random, but hopefully unless the observer stares at it,
they won't necessarily see that the ticking at any given moment is possibly inaccurate.

Lord Vetinari would likely approve.
