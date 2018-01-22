#        .text
       # .org 32768, 0
        .globl _start
_start:
        mov $077776, r6
        mov $0x8022, @$004 
        mov $0x8024, @$010 
        mov $0x8026, @$020 
        mov $0x8028, @$030 
        #jsr r7, @_notmain
        jsr r7, @$0x812c
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
