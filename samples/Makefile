PREFIX=/opt/pdp11-binutils/bin/
C_PREFIX=/opt/pdp11-gcc/bin/
PROG_P=pdp11-aout
CC=${C_PREFIX}/${PROG_P}-gcc
AS=${PREFIX}/${PROG_P}-as
OBJCOPY=${PREFIX}/${PROG_P}-objcopy
LD=${PREFIX}/${PROG_P}-ld

all: sample1.bin startup.bin bios

sample1.bin: sample1.S
	$(AS) $^ -o sample1.o
	$(OBJCOPY) -O binary sample1.o $@
	rm -f sample1.o

startup.bin: startup.S
	$(AS) $^ -o startup.o
	$(OBJCOPY) -O binary startup.o $@

bios: bios.o startup.o linker.T
	$(LD) -nostdlib -T linker.T -L${C_PREFIX}/../lib/gcc/pdp11-aout/6.4.0/ -o $@

startup.o: startup.S
	$(AS) $^ -o startup.o

#bios.o: bios.S
#	$(AS)  $^ -o $@
#	sed -e '/___main/d' -i bios.S

bios.o: bios.c
	$(CC) -ffreestanding -nostdlib -nostartfiles $^ -o $@
	

.PHONY: clean

clean:
	rm -f sample1.bin
