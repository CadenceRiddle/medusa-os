bits 16

section _ENTRY class=CODE

extern _cstart_
global entry

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
