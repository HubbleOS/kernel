.global _start
_start:
    ldr x30, =stack_top
    mov sp, x30
    bl kmain
    wfi
    b .


.global _terminate
_terminate: 
    wfi // ожидаем прерывание
    b .
