PREFIX=/opt/pdp11-binutils/bin/
C_PREFIX=/opt/pdp11-gcc/bin/
PROG_P=pdp11-aout
CC=${C_PREFIX}/${PROG_P}-gcc
AS=${PREFIX}/${PROG_P}-as
OBJCOPY=${PREFIX}/${PROG_P}-objcopy
LD=${PREFIX}/${PROG_P}-ld

all: sample1.bin startup.bin bios vertline vertline_vsync

sample1.bin: sample1.S
	$(AS) $^ -o sample1.o
	$(OBJCOPY) -O binary sample1.o $@
	rm -f sample1.o

startup.bin: startup.S
	$(AS) $^ -o startup.o
	$(OBJCOPY) -O binary startup.o $@

bios: bios.out
	$(OBJCOPY) -O binary bios.out $@
#	mv $^ $@
bios.out: bios.o startup.o linker.bios.T
	$(LD) -nostdlib -T linker.bios.T -L${C_PREFIX}/../lib/gcc/pdp11-aout/6.4.0/ -o $@

bios.o: bios.c
	$(CC) -fPIC -ffreestanding -nostdlib -nostartfiles -Wno-implicit-function-declaration -Wno-int-conversion $^ -o $@
	

vertline: vertline.out
	$(OBJCOPY) -O binary vertline.out $@
#	mv $^ $@
vertline.out: vertline.o startup.o linker.vertline.T
	$(LD) -nostdlib -T linker.vertline.T -L${C_PREFIX}/../lib/gcc/pdp11-aout/6.4.0/ -o $@
vertline.o: vertline.c
	$(CC) -fPIC -ffreestanding -nostdlib -nostartfiles -Wno-implicit-function-declaration -Wno-int-conversion $^ -o $@
	
vertline_vsync: vertline_vsync.out
	$(OBJCOPY) -O binary vertline_vsync.out $@
#	mv $^ $@
vertline_vsync.out: vertline_vsync.o startup.o linker.vertline_vsync.T
	$(LD) -nostdlib -T linker.vertline_vsync.T -L${C_PREFIX}/../lib/gcc/pdp11-aout/6.4.0/ -o $@
vertline_vsync.o: vertline.c
	$(CC) -DVSYNC -fPIC -ffreestanding -nostdlib -nostartfiles -Wno-implicit-function-declaration -Wno-int-conversion $^ -o $@
	

startup.o: startup.S
	$(AS) $^ -o startup.o

#bios.o: bios.S
#	$(AS)  $^ -o $@
#	sed -e '/___main/d' -i bios.S

.PHONY: clean

clean:
	rm -f sample1.bin
