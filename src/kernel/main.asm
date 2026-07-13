bits 16

section _ENTRY class=CODE

extern _kernel_main
global entry

entry:
    cli
    mov ax, es
    mov di, ax
    mov si, bx

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0FFFEh
    mov bp, sp
    sti

    push di
    push si
    call _kernel_main
    add sp, 4

    call enter_protected_mode

.halt:
    cli
    hlt
    jmp .halt

section _TEXT class=CODE

global _kernel_putchar
_kernel_putchar:
    push bp
    mov bp, sp

    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, 0
    int 10h

    mov sp, bp
    pop bp
    ret

enter_protected_mode:
    cli

    xor eax, eax
    mov ax, cs
    shl eax, 4
    mov bx, ax
    mov [gdt_code + 2], bx
    shr eax, 16
    mov [gdt_code + 4], al
    mov [gdt_code + 7], ah

    xor eax, eax
    mov ax, cs
    shl eax, 4
    add eax, gdt_start
    mov [gdt_descriptor + 2], eax

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEGMENT:protected_mode_entry

bits 32
protected_mode_entry:
    mov ax, CODE_SEGMENT
    mov ds, ax

    mov ax, DATA_SEGMENT
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 090000h

    mov esi, protected_mode_message
    mov edi, 0B8000h + 160 * 10
    mov ah, 0Ah

.print:
    lodsb
    test al, al
    jz .done
    mov [es:edi], ax
    add edi, 2
    jmp .print

.done:
    hlt
    jmp .done

bits 16

gdt_start:
    dq 0

gdt_code:
    dw 0FFFFh
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0FFFFh
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd 0

CODE_SEGMENT equ gdt_code - gdt_start
DATA_SEGMENT equ gdt_data - gdt_start

protected_mode_message: db 'Entered 32-bit protected mode', 0
