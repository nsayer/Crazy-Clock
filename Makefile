#
# To use this, you use 'make init TYPE={the kind of clock you want}'
# That will fuse, flash and seed the chip.
#

all: calibrate.hex normal.hex crazy.hex early.hex lazy.hex martian.hex sidereal.hex tidal.hex vetinari.hex warpy.hex wavy.hex whacky.hex tuney.hex

# Change this as appropriate! Don't screw it up!

# The clock is a 32.768 kHz crystal.
OPTS = -DF_CPU=32768L

# Change this to pick the correct programmer you're using
PROG = usbtiny

# Change this if you're not using a Tiny45
CHIP = attiny45

# The SPI clock must be less than the system clock divided by 6. A -B argument of 250 should
# yield an SPI clock of around 4 kHz, which is fine. If your programmer doesn't respect -B,
# then you will have to find some other way to insure ISP operations don't go too fast.
SPICLOCK = 250

CC = avr-gcc
OBJCPY = avr-objcopy
AVRDUDE = avrdude

CFLAGS = -Os -g -mmcu=$(CHIP) -std=c99 $(OPTS) -ffreestanding -Wall

DUDE_OPTS = -c $(PROG) -p $(CHIP) -B $(SPICLOCK)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

%.hex: %.elf
	$(OBJCPY) -j .text -j .data -O ihex $^ $@

# Calibrate is special - it has its own main()
calibrate.elf: calibrate.o
	$(CC) $(CFLAGS) -o $@ $^

%.elf: %.o base.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *.elf *.hex test-*


# The controller is fused for the extra-low frequency oscillator, no prescaling, and preserve
# the EEPROM content over code updates (to preserve the trim factor and PRNG seed).
fuse:
	$(AVRDUDE) $(DUDE_OPTS) -U lfuse:w:0xe6:m -U hfuse:w:0xd7:m -U efuse:w:0xff:m

flash: $(TYPE).hex
	$(AVRDUDE) $(DUDE_OPTS) -U flash:w:$(TYPE).hex

# This will perturb the stored PRNG seed.
seed:
	dd if=/dev/urandom bs=4 count=1 of=seedfile
	$(AVRDUDE) $(DUDE_OPTS) -U eeprom:w:seedfile:r
	rm -f seedfile

init: fuse flash seed

test:
	gcc -c -DUNIT_TEST -O -o test-$(TYPE).o $(TYPE).c
	gcc -c -O test.c
	gcc -o test-$(TYPE) test.o test-$(TYPE).o
