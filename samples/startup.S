        .globl _start
_start:
        mov r6, $077777
        mov @$004, _time
        mov @$010, _ill
        mov @$020, _iot
        mov @$030, _emt
        jsr r7, _notmain
        halt
        .even
_time:
        halt
        .even
_ill:
        halt
        .even
_iot:
        halt
        .even
_emt:
        halt
        .even
        .globl __main
__main:
        rts pc
