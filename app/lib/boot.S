
.section .text
.global _start
_start:
jal main
loop:
jal loop

.global __syscall

__syscall:
    mv a7,a0
    mv a0,a1
    mv a1,a2
    mv a2,a3
    mv a3,a4
    mv a4,a5
    ecall
    ret