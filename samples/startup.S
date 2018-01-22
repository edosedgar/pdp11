#        .text
       # .org 32768, 0
        .globl _start
_start:
        mov r6, $077777
        mov @$004, $0x8022
        mov @$010, $0x8024
        mov @$020, $0x8026
        mov @$030, $0x8028
        jsr r7, $0x8066
        halt
        .even
#        .globl _time
_time:
        halt
        .even
#        .globl _ill
_ill:
        halt
        .even
#        .globl _iot
_iot:
        halt
        .even
#        .globl _emt
_emt:
        halt
        .even
#        .globl __main
__main:
        rts pc
