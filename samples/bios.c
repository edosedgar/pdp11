#define VRAM 0x4000

void notmain(void) {
        short* vram = VRAM;
        set_bit(vram, 1, 1);
        delay(0x1);
        return;
}

void delay(short time) {
        while (time != 0) {
                short tmp1 = 0xF;
                while (tmp1 != 0) tmp1--;
                time--;
        }
}

void set_bit(short* vram, short x, short y) {
        short ind = y * 16 + (x / 16);
        short old = *(vram + ind);
        short new = old | (1 << (x & 0x000F)); 
        *(vram + ind)  = new;
}

void clr_bit(short* vram, short x, short y) {
        short ind = y * 16 + (x / 16);
        short old = *(vram + ind);
        short new = old & ~(1 << (x & 0x000F)); 
        *(vram + ind)  = new;
}


