#
# To use this, you use 'make init TYPE={the kind of clock you want}'
# That will fuse, flash and seed the chip.
#
# BE CAREFUL! Make sure that the fuse: target is set to the correct
# alternative for the crystal installed in your hardware. If you fuse
# the chip wrong, you will BRICK it!

all: normal.hex crazy.hex lazy.hex martian.hex sidereal.hex tidal.hex vetinari.hex warpy.hex wavy.hex whacky.hex

# Change this as appropriate! Don't screw it up!

# Pick these two for a 32.768 kHz crystal.
#fuse: fuse32k
#OPTS = -DTHIRTYTWO_KHZ_CLOCK

# Pick these two for a 4.00 MHz crystal
fuse: fuse4m
OPTS = -DFOUR_MHZ_CLOCK

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

%.elf: %.o base.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o *.elf *.hex

# The 4 MHz variant is fused for the 1-8 MHz oscillator and divide-by-8.
# The actual code will set that to divide-by-32, though, and that value
# is not cleared by a RESET, so the system clock will be 125 kHz.
fuse4m:
	$(AVRDUDE) $(DUDE_OPTS) -U lfuse:w:0x7d:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

# The 32 kHz variant is fused for the extra-low frequency oscillator and no prescaling.
fuse32k:
	$(AVRDUDE) $(DUDE_OPTS) -U lfuse:w:0xe6:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

flash: $(TYPE).hex
	$(AVRDUDE) $(DUDE_OPTS) -U flash:w:$(TYPE).hex

# This will perturb the stored PRNG seed.
seed:
	dd if=/dev/urandom bs=4 count=1 of=seedfile
	$(AVRDUDE) $(DUDE_OPTS) -U eeprom:w:seedfile:r
	rm -f seedfile

init: fuse flash seed

