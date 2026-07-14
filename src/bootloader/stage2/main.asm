bits 16

section _ENTRY class=CODE

extern _cstart_
global entry

; Sets up a real-mode C environment for stage 2 and calls the C loader entry.
entry:
    cli
    push cs
    pop ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti 

    xor dh, dh
    push dx
    call _cstart_
    
    cli
    hlt
