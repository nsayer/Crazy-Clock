#
# To use this, you use 'make init TYPE={the kind of clock you want}'
# That will fuse, flash and seed the chip.
#
# BE CAREFUL! Make sure that the fuse: target is set to the correct
# alternative for the crystal installed in your hardware. If you fuse
# the chip wrong, you will BRICK it!
#
# While you're at it, you should check the #defines in base.c. In particular,
# TEN_BASED_CLOCK and THIRTYTWO_KHZ_CLOCK

all: normal.hex crazy.hex lazy.hex martian.hex sidereal.hex vetinari.hex warpy.hex wavy.hex whacky.hex

# Change this as appropriate! Don't screw it up!
# fuse: fuse32k
fuse: fuse4m

# Change this to pick the correct programmer you're using
PROG = usbtiny

# Change this if you're not using a Tiny45
CHIP = attiny45

# Set this to 40 (for 32 kHz) if you're using a 4 MHz crystal. Set this to 250 (for 4 kHz) for the 32 kHz crystal
# It's safe (but slow) to just always use 250.
SPICLOCK = 250

CC = avr-gcc
OBJCPY = avr-objcopy
AVRDUDE = avrdude
CFLAGS = -Os -g -mmcu=$(CHIP) -std=c99

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.hex: %.elf
	$(OBJCPY) -j .text -j .data -O ihex $^ $@

%.elf: %.o base.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *.elf *.hex

# The 4 MHz variant is fused for the 1-8 MHz oscillator and divide-by-8.
# The actual code will set that to divide-by-32, though, and that value
# is not cleared by a RESET, so the system clock will be 125 kHz.
fuse4m:
	$(AVRDUDE) -c $(PROG) -p $(CHIP) -B $(SPICLOCK) -U lfuse:w:0x7d:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

# The 32 kHz variant is fused for the extra-low frequency oscillator and no prescaling.
fuse32k:
	$(AVRDUDE) -c $(PROG) -p $(CHIP) -B $(SPICLOCK) -U lfuse:w:0xe6:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

flash: $(TYPE).hex
	$(AVRDUDE) -c $(PROG) -p $(CHIP) -B $(SPICLOCK) -U flash:w:$(TYPE).hex

# This will perturb the stored PRNG seed.
seed:
	dd if=/dev/urandom bs=4 count=1 of=seedfile
	$(AVRDUDE) -c $(PROG) -p $(CHIP) -B $(SPICLOCK) -U eeprom:w:seedfile:r
	rm -f seedfile

init: fuse flash seed

