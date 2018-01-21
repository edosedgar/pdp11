# 1 "bios.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "bios.S"
 .text

 .even
 .globl _delay
_delay:
 mov r5, -(sp)
 mov sp, r5
 add $-02, sp
 br L_2
L_5:
 mov $-01, -02(r5)
 br L_3
L_4:
 mov -02(r5), r0
 dec r0
 mov r0, -02(r5)
L_3:
 tst -02(r5)
 bne L_4
 mov 04(r5), r0
 dec r0
 mov r0, 04(r5)
L_2:
 tst 04(r5)
 bne L_5
 nop
 mov r5, sp
 mov (sp)+, r5
 rts pc
 .even
 .globl _main
_main:
 setd
 seti
 mov r5, -(sp)
 mov sp, r5
 jsr pc, ___main
 mov $07777, -(sp)
 jsr pc, _delay
 add $02, sp
 clr r0
 mov (sp)+, r5
 rts pc
