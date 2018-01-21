ENTRY(_start)
SECTIONS {
        . = 0x8000;
        .text : {
                startup.o (.text);
                bios.o (.text);
        }
}
OUTPUT(binary)
OUTPUT(bios.bin)
