
void delay(short time) {
        while (time != 0) {
                short tmp1 = 0xFFFF;
                while (tmp1 != 0) tmp1--;
                time--;
        }
}

void notmain(void) {
        delay(0xFFF);
        return;
}
