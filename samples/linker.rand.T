ENTRY(_start)
SECTIONS {
        .text 0x8000 : {
                startup.o (.text);
                rand.o (.text);
        }
}
