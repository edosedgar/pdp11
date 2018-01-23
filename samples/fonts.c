#define VRAM 0x4000
#define VRAM2 0x6000
#define VRAM_CONTROL 0xF000

void notmain(void) {
        volatile short* vram = VRAM;
        demo();
        return;
}

//void hor_line();

short abs(short x) {
        if (x > 0) {
                return x;
        }
        return 0 - x;
}

void set_bit(short* vram, short x, short y, short v) {
        short ind = y * 16 + (x / 16);
        short old = *(vram + ind);
        short new;
        if (v == 1) {
                new = old | (1 << (x & 0x000F));
        } else {
                new = old & ~(1 << (x & 0x000F));
        }
        *(vram + ind)  = new;
}
/*
void clr_bit(short* vram, short x, short y) {
        short ind = y * 16 + (x / 16);
        short old = *(vram + ind);
        short new = old & ~(1 << (x & 0x000F));
        *(vram + ind)  = new;
}
*/
void draw_line(short x1, short y1, short x2, short y2, short v) {
    const short deltaX = abs(x2 - x1);
    const short deltaY = abs(y2 - y1);
    const short signX = x1 < x2 ? 1 : -1;
    const short signY = y1 < y2 ? 1 : -1;
    short* vram = VRAM;

    //
    short error = deltaX - deltaY;
    //
    set_bit(vram, x2, y2, v);
    while(x1 != x2 || y1 != y2)
   {
        set_bit(vram, x1, y1, v);
        const short error2 = error * 2;
        //
        if(error2 > -deltaY)
        {
            error -= deltaY;
            x1 += signX;
        }
        if(error2 < deltaX)
        {
            error += deltaX;
            y1 += signY;
        }
    }

}

volatile void delay(short time) {
        while (time != 0) {
                short tmp1 = 0xFF;
                while (tmp1 != 0) tmp1--;
                time--;
        }
}

void vert_line(short* vram, short X, short v) {
        short i = 0;
        for (i = 0; i < 128; i++) {
                vram[i*16 + X] = v;
        }
}

void flip(short v) {
        short* vram_control = VRAM_CONTROL;
        *vram_control = v;
}

void demo() {
        short i = 0;
        short* vram = VRAM;
        unsigned short xchar[16] = 
        { 0x8001, 0x4002, 0x4002, 0x2004, 0x2004}
        while (1) {
#ifdef VSYNC
                flip(1);
                for (i = 0; i < 16; i++) {
                        vert_line(vram + (0x2000) * (i&1), i, 0xFFFF);
                        flip((i & 1));
                        delay(10000);
                        flip(!(i & 1));
                        vert_line(vram + (0x2000) * (i&1), i, 0);
                        flip((i & 1));
                }
#else
                for (i = 0; i < 16; i++) {
                        vert_line(vram, i, 0xFFFF);
                        delay(10000);
                        vert_line(vram, i, 0);
                }
#endif

        }
}
