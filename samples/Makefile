PREFIX=/opt/pdp11-binutils/bin/
PROG_P=pdp11-aout
AS=${PREFIX}/${PROG_P}-as
OBJCOPY=${PREFIX}/${PROG_P}-objcopy

sample1.bin: sample1.S
	$(AS) $^ -o sample1.o
	$(OBJCOPY) -O binary sample1.o $@
	rm -f sample1.o

.PHONY: clean

clean:
	rm -f sample1.bin
