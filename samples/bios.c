#define VRAM 0x4000

void notmain(void) {
        short* vram = VRAM;
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

void drawLine(short x1, short y1, short x2, short y2) {
    const short deltaX = abs(x2 - x1);
    const short deltaY = abs(y2 - y1);
    const short signX = x1 < x2 ? 1 : -1;
    const short signY = y1 < y2 ? 1 : -1;
    short* vram = VRAM;
        
    //
    short error = deltaX - deltaY;
    //
    set_bit(vram, x2, y2);
    while(x1 != x2 || y1 != y2) 
   {
        set_bit(vram, x1, y1);
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

void delay(short time) {
        while (time != 0) {
                short tmp1 = 0xF;
                while (tmp1 != 0) tmp1--;
                time--;
        }
}

void demo() {
        short i = 0;
        short* vram = VRAM;
        drawLine(0, 0, 255, 255);
/*        for (i = 0; i < 256; i++) {
                set_bit(vram, i, 0);
        }
*/
}
