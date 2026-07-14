bits 16
org 10000h

KERNEL_BASE equ 10000h
KERNEL32_OFFSET equ 1000h
KERNEL32_ENTRY equ KERNEL_BASE + KERNEL32_OFFSET

CODE_SEGMENT equ gdt_code - gdt_start
DATA_SEGMENT equ gdt_data - gdt_start

entry:
    cli

    mov dx, cs
    mov ds, dx

    xor eax, eax
    mov ax, es
    shl eax, 4
    movzx ebx, bx
    add eax, ebx
    mov [boot_info_physical - KERNEL_BASE], eax

    mov ax, dx
    mov es, ax
    mov ss, ax
    mov sp, 0FFFEh
    mov bp, sp

    lgdt [gdt_descriptor - KERNEL_BASE]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword CODE_SEGMENT:protected_mode_entry

bits 32
protected_mode_entry:
    mov ax, DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 090000h

    push dword [boot_info_physical]
    call KERNEL32_ENTRY
    add esp, 4

.halt:
    cli
    hlt
    jmp .halt

align 8
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
    dd gdt_start

boot_info_physical: dd 0

times KERNEL32_OFFSET - ($ - $$) db 0
incbin KERNEL32_BIN
