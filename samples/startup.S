#        .text
       # .org 32768, 0
        .globl _start
_start:
        mov $0x3ffc, r6
        mov $0x8026, @$004 
        mov $0x8028, @$010 
        mov $0x802a, @$020 
        mov $0x802c, @$030 
        jsr r7, @$0x8030
        halt
        jsr r7, @_notmain
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
