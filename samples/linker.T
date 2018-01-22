ENTRY(_start)
SECTIONS {
        .text 0x8000 : {
                startup.o (.text);
                bios.o (.text);
        }
}
OUTPUT(binary)
OUTPUT(bios.bin)
