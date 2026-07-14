bits 32

extern kernel_exception_handler

global interrupt_stub_table

%macro ISR_NO_ERROR 1
global interrupt_stub_%1
interrupt_stub_%1:
    push dword 0
    push dword %1
    jmp interrupt_common
%endmacro

%macro ISR_ERROR 1
global interrupt_stub_%1
interrupt_stub_%1:
    push dword %1
    jmp interrupt_common
%endmacro

ISR_NO_ERROR 0
ISR_NO_ERROR 1
ISR_NO_ERROR 2
ISR_NO_ERROR 3
ISR_NO_ERROR 4
ISR_NO_ERROR 5
ISR_NO_ERROR 6
ISR_NO_ERROR 7
ISR_ERROR 8
ISR_NO_ERROR 9
ISR_ERROR 10
ISR_ERROR 11
ISR_ERROR 12
ISR_ERROR 13
ISR_ERROR 14
ISR_NO_ERROR 15
ISR_NO_ERROR 16
ISR_ERROR 17
ISR_NO_ERROR 18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_ERROR 21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_ERROR 29
ISR_ERROR 30
ISR_NO_ERROR 31

interrupt_common:
    pusha

    mov eax, [esp + 32]
    mov edx, [esp + 36]
    push edx
    push eax
    call kernel_exception_handler

.halt:
    cli
    hlt
    jmp .halt

section .rodata
interrupt_stub_table:
    dd interrupt_stub_0
    dd interrupt_stub_1
    dd interrupt_stub_2
    dd interrupt_stub_3
    dd interrupt_stub_4
    dd interrupt_stub_5
    dd interrupt_stub_6
    dd interrupt_stub_7
    dd interrupt_stub_8
    dd interrupt_stub_9
    dd interrupt_stub_10
    dd interrupt_stub_11
    dd interrupt_stub_12
    dd interrupt_stub_13
    dd interrupt_stub_14
    dd interrupt_stub_15
    dd interrupt_stub_16
    dd interrupt_stub_17
    dd interrupt_stub_18
    dd interrupt_stub_19
    dd interrupt_stub_20
    dd interrupt_stub_21
    dd interrupt_stub_22
    dd interrupt_stub_23
    dd interrupt_stub_24
    dd interrupt_stub_25
    dd interrupt_stub_26
    dd interrupt_stub_27
    dd interrupt_stub_28
    dd interrupt_stub_29
    dd interrupt_stub_30
    dd interrupt_stub_31

section .note.GNU-stack noalloc noexec nowrite progbits
