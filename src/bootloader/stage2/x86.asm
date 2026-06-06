bits 16

section _TEXT class=CODE

global _x86_Video_WriteCharTeletype_
_x86_Video_WriteCharTeletype_:
    push bp
    mov bp, sp

    mov ah, 0Eh
    mov bh, dl

    int 10h

    mov sp, bp
    pop bp
    ret
